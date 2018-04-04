#pragma once

#include <nall/dsp/iir/one-pole.hpp>
#include <nall/dsp/iir/biquad.hpp>
#include <nall/dsp/resampler/cubic.hpp>

namespace Emulator {

struct Interface;
struct Audio;
struct Filter;
struct Stream;

struct Audio {
  auto reset(maybe<uint> channels = nothing, maybe<double> frequency = nothing) -> void;
  auto setInterface(Interface* interface) -> void;

  auto setFrequency(double frequency) -> void;
  auto setVolume(double volume) -> void;
  auto setBalance(double balance) -> void;
  auto setReverb(bool enabled) -> void;

  auto createStream(uint channels, double frequency) -> shared_pointer<Stream>;

private:
  auto process() -> void;

  Interface* interface = nullptr;
  vector<shared_pointer<Stream>> streams;

  uint channels = 0;
  double frequency = 0.0;

  double volume = 1.0;
  double balance = 0.0;

  bool reverbEnable = false;
  vector<vector<queue<double>>> reverb;

  friend class Stream;
};

struct Filter {
  enum class Order : uint { First, Second };
  enum class Type : uint { LowPass, HighPass };

  Order order;
  DSP::IIR::OnePole onePole;  //first-order
  DSP::IIR::Biquad biquad;    //second-order
};

struct Stream {
  auto reset(uint channels, double inputFrequency, double outputFrequency) -> void;

  auto setFrequency(double inputFrequency, maybe<double> outputFrequency = nothing) -> void;

  auto addFilter(Filter::Order order, Filter::Type type, double cutoffFrequency, uint passes = 1) -> void;

  auto pending() const -> bool;
  auto read(double samples[]) -> uint;
  auto write(const double samples[]) -> void;

  template<typename... P> auto sample(P&&... p) -> void {
    double samples[sizeof...(P)] = {forward<P>(p)...};
    write(samples);
  }

private:
  struct Channel {
    vector<Filter> filters;
    DSP::Resampler::Cubic resampler;
  };
  vector<Channel> channels;
  double inputFrequency;
  double outputFrequency;

  friend class Audio;
};

extern Audio audio;

}
