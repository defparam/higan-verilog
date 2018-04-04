#include <alsa/asoundlib.h>

struct AudioALSA : Audio {
  AudioALSA() { initialize(); }
  ~AudioALSA() { terminate(); }

  auto availableDevices() -> string_vector {
    return queryDevices();
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
  auto device() -> string { return _device; }
  auto blocking() -> bool { return _blocking; }
  auto channels() -> uint { return 2; }
  auto frequency() -> double { return _frequency; }
  auto latency() -> uint { return _latency; }

  auto setDevice(string device) -> bool {
    if(_device == device) return true;
    _device = device;
    return initialize();
  }

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
    if(!ready()) return;

    _buffer[_offset]  = (uint16_t)sclamp<16>(samples[0] * 32767.0) <<  0;
    _buffer[_offset] |= (uint16_t)sclamp<16>(samples[1] * 32767.0) << 16;
    if(++_offset < _periodSize) return;

    snd_pcm_sframes_t available;
    do {
      available = snd_pcm_avail_update(_interface);
      if(available < 0) snd_pcm_recover(_interface, available, 1);
      if(available < _offset) {
        if(!_blocking) {
          _offset = 0;
          return;
        }
        int error = snd_pcm_wait(_interface, -1);
        if(error < 0) snd_pcm_recover(_interface, error, 1);
      }
    } while(available < _offset);

    uint32_t* output = _buffer;
    int i = 4;

    while(_offset > 0 && i--) {
      snd_pcm_sframes_t written = snd_pcm_writei(_interface, output, _offset);
      if(written < 0) {
        //no samples written
        snd_pcm_recover(_interface, written, 1);
      } else if(written <= _offset) {
        _offset -= written;
        output += written;
      }
    }

    if(i < 0) {
      if(_buffer == output) {
        _offset--;
        output++;
      }
      memory::move(_buffer, output, _offset * sizeof(uint32_t));
    }
  }

private:
  auto initialize() -> bool {
    terminate();

    string device = "default";
    if(queryDevices().find(_device)) device = _device;
    if(snd_pcm_open(&_interface, device, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK) < 0) return terminate(), false;

    uint rate = (uint)_frequency;
    uint bufferTime = _latency * 1000;
    uint periodTime = _latency * 1000 / 4;

    snd_pcm_hw_params_t* hardwareParameters;
    snd_pcm_hw_params_alloca(&hardwareParameters);
    if(snd_pcm_hw_params_any(_interface, hardwareParameters) < 0) return terminate(), false;

    if(snd_pcm_hw_params_set_access(_interface, hardwareParameters, SND_PCM_ACCESS_RW_INTERLEAVED) < 0
    || snd_pcm_hw_params_set_format(_interface, hardwareParameters, SND_PCM_FORMAT_S16_LE) < 0
    || snd_pcm_hw_params_set_channels(_interface, hardwareParameters, 2) < 0
    || snd_pcm_hw_params_set_rate_near(_interface, hardwareParameters, &rate, 0) < 0
    || snd_pcm_hw_params_set_period_time_near(_interface, hardwareParameters, &periodTime, 0) < 0
    || snd_pcm_hw_params_set_buffer_time_near(_interface, hardwareParameters, &bufferTime, 0) < 0
    ) return terminate(), false;

    if(snd_pcm_hw_params(_interface, hardwareParameters) < 0) return terminate(), false;
    if(snd_pcm_get_params(_interface, &_bufferSize, &_periodSize) < 0) return terminate(), false;

    snd_pcm_sw_params_t* softwareParameters;
    snd_pcm_sw_params_alloca(&softwareParameters);
    if(snd_pcm_sw_params_current(_interface, softwareParameters) < 0) return terminate(), false;
    if(snd_pcm_sw_params_set_start_threshold(_interface, softwareParameters,
      (_bufferSize / _periodSize) * _periodSize) < 0
    ) return terminate(), false;
    if(snd_pcm_sw_params(_interface, softwareParameters) < 0) return terminate(), false;

    _buffer = new uint32_t[_periodSize]();
    _offset = 0;
    return _ready = true;
  }

  auto terminate() -> void {
    _ready = false;

    if(_interface) {
    //snd_pcm_drain(_interface);  //prevents popping noise; but causes multi-second lag
      snd_pcm_close(_interface);
      _interface = nullptr;
    }

    if(_buffer) {
      delete[] _buffer;
      _buffer = nullptr;
    }
  }

  auto queryDevices() -> string_vector {
    string_vector devices;

    char** list;
    if(snd_device_name_hint(-1, "pcm", (void***)&list) == 0) {
      uint index = 0;
      while(list[index]) {
        char* deviceName = snd_device_name_get_hint(list[index], "NAME");
        if(deviceName) devices.append(deviceName);
        free(deviceName);
        index++;
      }
    }

    snd_device_name_free_hint((void**)list);
    return devices;
  }

  bool _ready = false;
  string _device;
  bool _blocking = true;
  double _frequency = 48000.0;
  uint _latency = 40;

  snd_pcm_t* _interface = nullptr;
  snd_pcm_uframes_t _bufferSize;
  snd_pcm_uframes_t _periodSize;

  uint32_t* _buffer = nullptr;
  uint _offset = 0;
};
