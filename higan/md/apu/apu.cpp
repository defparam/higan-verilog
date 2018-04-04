#include <md/md.hpp>

namespace MegaDrive {

APU apu;
#include "bus.cpp"
#include "serialization.cpp"

auto APU::Enter() -> void {
  while(true) scheduler.synchronize(), apu.main();
}

auto APU::main() -> void {
  if(!state.enabled) {
    return step(1);
  }

  if(state.nmiLine) {
    state.nmiLine = 0;  //edge-sensitive
    irq(0, 0x0066, 0xff);
  }

  if(state.intLine) {
    //level-sensitive
    irq(1, 0x0038, 0xff);
  }

  instruction();
}

auto APU::step(uint clocks) -> void {
  Thread::step(clocks);
  synchronize(cpu);
}

auto APU::synchronizing() const -> bool {
  return scheduler.synchronizing();
}

auto APU::setNMI(bool value) -> void {
  state.nmiLine = value;
}

auto APU::setINT(bool value) -> void {
  state.intLine = value;
}

auto APU::enable(bool value) -> void {
  //68K cannot disable the Z80 without bus access
  if(!bus->granted() && !value) return;
  if(state.enabled && !value) reset();
  state.enabled = value;
}

auto APU::power(bool reset) -> void {
  Z80::bus = this;
  Z80::power();
  bus->grant(false);
  create(APU::Enter, system.frequency() / 15.0);

  if(!reset) memory::fill(ram, sizeof ram);
  state = {};
}

auto APU::reset() -> void {
  Z80::power();
  bus->grant(false);
  create(APU::Enter, system.frequency() / 15.0);
  state = {};
}

}
