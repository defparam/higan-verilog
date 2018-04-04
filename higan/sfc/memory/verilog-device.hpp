
struct VerilogIf {
  VerilogIf();
  auto read(uint24 addr, uint8 data) -> uint8;
  auto write(uint24 addr, uint8 data) -> uint8;
private:
  Vmemsnoop* memsnoop;
  VerilatedVcdC* tfp;
  uint64 main_time;
  auto quiesce(int steps) -> void;
  auto modelread(uint24 addr, uint8 data) -> uint8;
  auto modelwrite(uint24 addr, uint8 data) -> uint8;
  auto step(int steps) -> void;
};

extern VerilogIf ver;