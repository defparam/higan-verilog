#include <sfc/sfc.hpp>

namespace SuperFamicom {

#include "bus.cpp"
#include "core.cpp"
#include "memory.cpp"
#include "io.cpp"
#include "timing.cpp"
#include "serialization.cpp"
SuperFX superfx;

auto SuperFX::Enter() -> void {
  while(true) scheduler.synchronize(), superfx.main();
}

auto SuperFX::main() -> void {
  if(regs.sfr.g == 0) return step(6);

  instruction(peekpipe());

  if(regs.r[14].modified) {
    regs.r[14].modified = false;
    updateROMBuffer();
  }

  if(regs.r[15].modified) {
    regs.r[15].modified = false;
  } else {
    regs.r[15]++;
  }
}

auto SuperFX::unload() -> void {
  rom.reset();
  ram.reset();
}

auto SuperFX::power() -> void {
  GSU::power();
  create(SuperFX::Enter, system.cpuFrequency());

  rom.writeProtect(true);
  ram.writeProtect(false);

  romMask = rom.size() - 1;
  ramMask = ram.size() - 1;

  for(uint n : range(512)) cache.buffer[n] = 0x00;
  for(uint n : range(32)) cache.valid[n] = false;
  for(uint n : range(2)) {
    pixelcache[n].offset = ~0;
    pixelcache[n].bitpend = 0x00;
  }

  regs.romcl = 0;
  regs.romdr = 0;

  regs.ramcl = 0;
  regs.ramar = 0;
  regs.ramdr = 0;
}

}
