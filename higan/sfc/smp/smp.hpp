//Sony CXP1100Q-1

struct SMP : Processor::SPC700, Thread {
  uint8 iplrom[64];

  //smp.cpp
  auto synchronizing() const -> bool override;

  auto portRead(uint2 port) const -> uint8;
  auto portWrite(uint2 port, uint8 data) -> void;

  auto main() -> void;
  auto load(Markup::Node) -> bool;
  auto power(bool reset) -> void;

  //serialization.cpp
  auto serialize(serializer&) -> void;

private:
  struct IO {
    //timing
    uint clockCounter;
    uint dspCounter;

    //external
    uint8 apu0;
    uint8 apu1;
    uint8 apu2;
    uint8 apu3;

    //$00f0
    uint1 timersDisable;
    uint1 ramWritable;
    uint1 ramDisable;
    uint1 timersEnable;
    uint2 externalWaitStates;
    uint2 internalWaitStates;

    //$00f1
    bool iplromEnable;

    //$00f2
    uint8 dspAddr;

    //$00f4-00f7
    uint8 cpu0;
    uint8 cpu1;
    uint8 cpu2;
    uint8 cpu3;

    //$00f8-00f9
    uint8 aux4;
    uint8 aux5;
  } io;

  static auto Enter() -> void;

  //memory.cpp
  auto ramRead(uint16 addr) -> uint8;
  auto ramWrite(uint16 addr, uint8 data) -> void;

  auto busRead(uint16 addr) -> uint8;
  auto busWrite(uint16 addr, uint8 data) -> void;

  auto idle() -> void override;
  auto read(uint16 addr) -> uint8 override;
  auto write(uint16 addr, uint8 data) -> void override;

  auto readDisassembler(uint16 addr) -> uint8 override;

  //timing.cpp
  template<uint Frequency> struct Timer {
    uint8 stage0;
    uint8 stage1;
    uint8 stage2;
    uint4 stage3;
    bool line;
    bool enable;
    uint8 target;

    auto step(uint clocks) -> void;
    auto synchronizeStage1() -> void;
  };

  Timer<128> timer0;
  Timer<128> timer1;
  Timer< 16> timer2;

  inline auto wait(maybe<uint16> address = nothing) -> void;
  inline auto step(uint clocks) -> void;
  inline auto stepTimers(uint clocks) -> void;
};

extern SMP smp;
