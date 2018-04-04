struct APU : Thread, IO {
  shared_pointer<Emulator::Stream> stream;

  static auto Enter() -> void;
  auto main() -> void;
  auto step(uint clocks) -> void;

  auto readIO(uint32 addr) -> uint8;
  auto writeIO(uint32 addr, uint8 byte) -> void;
  auto power() -> void;

  auto runsequencer() -> void;

  auto serialize(serializer&) -> void;

  uint clock;

  struct Registers {
    struct SoundBias {
      uint10 level;
      uint2 amplitude;
    } bias;
  } regs;

  struct Sweep {
    uint3 shift;
    uint1 direction;
    uint3 frequency;

    uint1 enable;
    uint1 negate;
    uint3 period;
  };

  struct Envelope {
    uint3 frequency;
    uint1 direction;
    uint4 volume;

    uint3 period;

    auto dacEnable() const -> bool { return volume || direction; }
  };

  struct Square {
    Envelope envelope;
    uint1 enable;
    uint6 length;
    uint2 duty;
    uint11 frequency;
    uint1 counter;
    uint1 initialize;

    int shadowfrequency;
    uint1 signal;
    uint4 output;
    uint period;
    uint3 phase;
    uint4 volume;

    auto run() -> void;
    auto clocklength() -> void;
    auto clockenvelope() -> void;
  };

  struct Square1 : Square {
    Sweep sweep;

    auto runsweep(bool update) -> void;
    auto clocksweep() -> void;
    auto read(uint addr) const -> uint8;
    auto write(uint addr, uint8 byte) -> void;
    auto power() -> void;
  } square1;

  struct Square2 : Square {
    auto read(uint addr) const -> uint8;
    auto write(uint addr, uint8 byte) -> void;
    auto power() -> void;
  } square2;

  struct Wave {
    uint1 mode;
    uint1 bank;
    uint1 dacenable;
    uint8 length;
    uint3 volume;
    uint11 frequency;
    uint1 counter;
    uint1 initialize;
    uint4 pattern[2 * 32];

    uint1 enable;
    uint4 output;
    uint5 patternaddr;
    uint1 patternbank;
    uint4 patternsample;
    uint period;

    auto run() -> void;
    auto clocklength() -> void;
    auto read(uint addr) const -> uint8;
    auto write(uint addr, uint8 byte) -> void;
    auto readram(uint addr) const -> uint8;
    auto writeram(uint addr, uint8 byte) -> void;
    auto power() -> void;
  } wave;

  struct Noise {
    Envelope envelope;
    uint6 length;
    uint3 divisor;
    uint1 narrowlfsr;
    uint4 frequency;
    uint1 counter;
    uint1 initialize;

    uint1 enable;
    uint15 lfsr;
    uint4 output;
    uint period;
    uint4 volume;

    auto divider() const -> uint;
    auto run() -> void;
    auto clocklength() -> void;
    auto clockenvelope() -> void;
    auto read(uint addr) const -> uint8;
    auto write(uint addr, uint8 byte) -> void;
    auto power() -> void;
  } noise;

  struct Sequencer {
    uint2 volume;
    uint3 lvolume;
    uint3 rvolume;
    uint1 lenable[4];
    uint1 renable[4];
    uint1 masterenable;

    uint12 base;
    uint3 step;
    int16 lsample;
    int16 rsample;

    uint10 loutput;
    uint10 routput;

    auto sample() -> void;

    auto read(uint addr) const -> uint8;
    auto write(uint addr, uint8 byte) -> void;
    auto power() -> void;
  } sequencer;

  struct FIFO {
    int8 samples[32];
    int8 active;
    int8 output;

    uint5 rdoffset;
    uint5 wroffset;
    uint6 size;

    uint1 volume;  //0 = 50%, 1 = 100%
    uint1 lenable;
    uint1 renable;
    uint1 timer;

    auto sample() -> void;
    auto read() -> void;
    auto write(int8 byte) -> void;
    auto reset() -> void;
    auto power() -> void;
  } fifo[2];
};

extern APU apu;
