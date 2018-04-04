#include <pulse/pulseaudio.h>

struct AudioPulseAudio : Audio {
  AudioPulseAudio() { initialize(); }
  ~AudioPulseAudio() { terminate(); }

  auto availableDevices() -> string_vector {
    return {"Default"};
  }

  auto availableFrequencies() -> vector<double> {
    return {44100.0, 48000.0, 96000.0};
  }

  auto availableLatencies() -> vector<uint> {
    return {20, 40, 60, 80, 100};
  }

  auto availableChannels() -> vector<uint> {
    return {2};
  }

  auto ready() -> bool { return _ready; }
  auto blocking() -> bool { return _blocking; }
  auto channels() -> uint { return 2; }
  auto frequency() -> double { return _frequency; }
  auto latency() -> uint { return _latency; }

  auto setBlocking(bool blocking) -> bool {
    if(_blocking == blocking) return true;
    _blocking = blocking;
    return true;
  }

  auto setFrequency(double frequency) -> bool {
    if(_frequency == frequency) return true;
    _frequency = frequency;
    return initialize();
  }

  auto setLatency(uint latency) -> bool {
    if(_latency == latency) return true;
    _latency = latency;
    return initialize();
  }

  auto output(const double samples[]) -> void {
    pa_stream_begin_write(_stream, (void**)&_buffer, &_period);
    _buffer[_offset]  = (uint16_t)sclamp<16>(samples[0] * 32767.0) <<  0;
    _buffer[_offset] |= (uint16_t)sclamp<16>(samples[1] * 32767.0) << 16;
    if((++_offset + 1) * pa_frame_size(&_specification) <= _period) return;

    while(true) {
      if(_first) {
        _first = false;
        pa_mainloop_iterate(_mainLoop, 0, nullptr);
      } else {
        pa_mainloop_iterate(_mainLoop, 1, nullptr);
      }
      uint length = pa_stream_writable_size(_stream);
      if(length >= _offset * pa_frame_size(&_specification)) break;
      if(!_blocking) {
        _offset = 0;
        return;
      }
    }

    pa_stream_write(_stream, (const void*)_buffer, _offset * pa_frame_size(&_specification), nullptr, 0LL, PA_SEEK_RELATIVE);
    _buffer = nullptr;
    _offset = 0;
  }

private:
  auto initialize() -> bool {
    terminate();

    _mainLoop = pa_mainloop_new();
    _context = pa_context_new(pa_mainloop_get_api(_mainLoop), "ruby::pulseAudio");
    pa_context_connect(_context, nullptr, PA_CONTEXT_NOFLAGS, nullptr);

    pa_context_state_t contextState;
    do {
      pa_mainloop_iterate(_mainLoop, 1, nullptr);
      contextState = pa_context_get_state(_context);
      if(!PA_CONTEXT_IS_GOOD(contextState)) return false;
    } while(contextState != PA_CONTEXT_READY);

    _specification.format = PA_SAMPLE_S16LE;
    _specification.channels = 2;
    _specification.rate = (uint)_frequency;
    _stream = pa_stream_new(_context, "audio", &_specification, nullptr);

    pa_buffer_attr bufferAttributes;
    bufferAttributes.maxlength = -1;
    bufferAttributes.tlength = pa_usec_to_bytes(_latency * PA_USEC_PER_MSEC, &_specification);
    bufferAttributes.prebuf = -1;
    bufferAttributes.minreq = -1;
    bufferAttributes.fragsize = -1;

    pa_stream_flags_t flags = (pa_stream_flags_t)(PA_STREAM_ADJUST_LATENCY | PA_STREAM_VARIABLE_RATE);
    pa_stream_connect_playback(_stream, nullptr, &bufferAttributes, flags, nullptr, nullptr);

    pa_stream_state_t streamState;
    do {
      pa_mainloop_iterate(_mainLoop, 1, nullptr);
      streamState = pa_stream_get_state(_stream);
      if(!PA_STREAM_IS_GOOD(streamState)) return false;
    } while(streamState != PA_STREAM_READY);

    _period = 960;
    _offset = 0;
    _first = true;
    return _ready = true;
  }

  auto terminate() -> void {
    _ready = false;

    if(_buffer) {
      pa_stream_cancel_write(_stream);
      _buffer = nullptr;
    }

    if(_stream) {
      pa_stream_disconnect(_stream);
      pa_stream_unref(_stream);
      _stream = nullptr;
    }

    if(_context) {
      pa_context_disconnect(_context);
      pa_context_unref(_context);
      _context = nullptr;
    }

    if(_mainLoop) {
      pa_mainloop_free(_mainLoop);
      _mainLoop = nullptr;
    }
  }

  bool _ready = false;
  bool _blocking = true;
  double _frequency = 48000.0;
  uint _latency = 40;

  uint32_t* _buffer = nullptr;
  size_t _period = 0;
  uint _offset = 0;

  pa_mainloop* _mainLoop = nullptr;
  pa_context* _context = nullptr;
  pa_stream* _stream = nullptr;
  pa_sample_spec _specification;
  bool _first = true;
};
