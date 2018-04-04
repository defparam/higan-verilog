#include <gb/gb.hpp>

namespace GameBoy {

Cartridge cartridge;
#include "mbc0/mbc0.cpp"
#include "mbc1/mbc1.cpp"
#include "mbc1m/mbc1m.cpp"
#include "mbc2/mbc2.cpp"
#include "mbc3/mbc3.cpp"
#include "mbc5/mbc5.cpp"
#include "mbc6/mbc6.cpp"
#include "mbc7/mbc7.cpp"
#include "mmm01/mmm01.cpp"
#include "huc1/huc1.cpp"
#include "huc3/huc3.cpp"
#include "tama/tama.cpp"
#include "serialization.cpp"

auto Cartridge::load() -> bool {
  information = {};
  rom = {};
  ram = {};
  rtc = {};
  mapper = &mbc0;
  accelerometer = false;
  rumble = false;

  if(Model::GameBoy()) {
    if(auto loaded = platform->load(ID::GameBoy, "Game Boy", "gb")) {
      information.pathID = loaded.pathID();
    } else return false;
  }

  if(Model::GameBoyColor()) {
    if(auto loaded = platform->load(ID::GameBoyColor, "Game Boy Color", "gbc")) {
      information.pathID = loaded.pathID();
    } else return false;
  }

  if(Model::SuperGameBoy()) {
    if(auto loaded = platform->load(ID::SuperGameBoy, "Game Boy", "gb")) {
      information.pathID = loaded.pathID();
    } else return false;
  }

  if(auto fp = platform->open(pathID(), "manifest.bml", File::Read, File::Required)) {
    information.manifest = fp->reads();
  } else return false;

  //todo: temporary hack so (type=) node lookup works; replace with a proper game/memory parser
  auto document = BML::unserialize(string{information.manifest}.replace("type: ", "type:"));
  information.title = document["game/label"].text();

  auto mapperID = document["game/board"].text();
  if(mapperID == "MBC0" ) mapper = &mbc0;
  if(mapperID == "MBC1" ) mapper = &mbc1;
  if(mapperID == "MBC1M") mapper = &mbc1m;
  if(mapperID == "MBC2" ) mapper = &mbc2;
  if(mapperID == "MBC3" ) mapper = &mbc3;
  if(mapperID == "MBC5" ) mapper = &mbc5;
  if(mapperID == "MBC6" ) mapper = &mbc6;
  if(mapperID == "MBC7" ) mapper = &mbc7;
  if(mapperID == "MMM01") mapper = &mmm01;
  if(mapperID == "HuC1" ) mapper = &huc1;
  if(mapperID == "HuC3" ) mapper = &huc3;
  if(mapperID == "TAMA" ) mapper = &tama;

  accelerometer = (bool)document["game/board/accelerometer"];
  rumble = (bool)document["game/board/rumble"];

  if(auto node = document["game/memory(type=ROM)"]) {
    rom.size = max(0x4000, node["size"].natural());
    rom.data = (uint8*)memory::allocate(rom.size, 0xff);
    if(auto name = node["name"].text()) {
      if(auto fp = platform->open(pathID(), name, File::Read, File::Required)) {
        fp->read(rom.data, min(rom.size, fp->size()));
      }
    }
  }

  if(auto node = document["game/memory(type=NVRAM)"]) {
    ram.size = node["size"].natural();
    ram.data = (uint8*)memory::allocate(ram.size, 0xff);
    if(auto name = node["name"].text()) {
      if(auto fp = platform->open(pathID(), name, File::Read, File::Optional)) {
        fp->read(ram.data, min(ram.size, fp->size()));
      }
    }
  }

  if(auto node = document["game/memory(type=RTC)"]) {
    rtc.size = node["size"].natural();
    rtc.data = (uint8*)memory::allocate(rtc.size, 0xff);
    if(auto name = node["name"].text()) {
      if(auto fp = platform->open(pathID(), name, File::Read, File::Optional)) {
        fp->read(rtc.data, min(rtc.size, fp->size()));
      }
    }
  }

  information.sha256 = Hash::SHA256(rom.data, rom.size).digest();
  return true;
}

auto Cartridge::save() -> void {
  auto document = BML::unserialize(string{information.manifest}.replace("type: ", "type:"));

  if(auto node = document["game/memory(type=NVRAM)"]) {
    if(auto name = node["name"].text()) {
      if(auto fp = platform->open(pathID(), name, File::Write)) {
        fp->write(ram.data, ram.size);
      }
    }
  }

  if(auto node = document["game/memory(type=RTC)"]) {
    if(auto name = node["name"].text()) {
      if(auto fp = platform->open(pathID(), name, File::Write)) {
        fp->write(rtc.data, rtc.size);
      }
    }
  }
}

auto Cartridge::unload() -> void {
  delete[] rom.data;
  delete[] ram.data;
  delete[] rtc.data;
  rom = {};
  ram = {};
  rtc = {};
}

auto Cartridge::readIO(uint16 addr) -> uint8 {
  if(addr == 0xff50) return 0xff;

  if(bootromEnable) {
    const uint8* data = nullptr;
    if(Model::GameBoy()) data = system.bootROM.dmg;
    if(Model::GameBoyColor()) data = system.bootROM.cgb;
    if(Model::SuperGameBoy()) data = system.bootROM.sgb;
    if(addr >= 0x0000 && addr <= 0x00ff) return data[addr];
    if(addr >= 0x0200 && addr <= 0x08ff && Model::GameBoyColor()) return data[addr - 0x100];
  }

  return mapper->read(addr);
}

auto Cartridge::writeIO(uint16 addr, uint8 data) -> void {
  if(bootromEnable && addr == 0xff50) {
    bootromEnable = false;
    return;
  }

  mapper->write(addr, data);
}

auto Cartridge::power() -> void {
  for(uint n = 0x0000; n <= 0x7fff; n++) bus.mmio[n] = this;
  for(uint n = 0xa000; n <= 0xbfff; n++) bus.mmio[n] = this;
  bus.mmio[0xff50] = this;

  bootromEnable = true;

  mapper->power();
}

auto Cartridge::second() -> void {
  mapper->second();
}

auto Cartridge::Memory::read(uint address) const -> uint8 {
  if(!size) return 0xff;
  if(address >= size) address %= size;
  return data[address];
}

auto Cartridge::Memory::write(uint address, uint8 byte) -> void {
  if(!size) return;
  if(address >= size) address %= size;
  data[address] = byte;
}

}
