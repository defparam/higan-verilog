//NES-GNROM
//NES-MHROM

struct NES_GxROM : Board {
  NES_GxROM(Markup::Node& document) : Board(document) {
    settings.mirror = document["board/mirror/mode"].text() == "vertical" ? 1 : 0;
  }

  auto readPRG(uint addr) -> uint8 {
    if(addr & 0x8000) return prgrom.read((prgBank << 15) | (addr & 0x7fff));
    return cpu.mdr();
  }

  auto writePRG(uint addr, uint8 data) -> void {
    if(addr & 0x8000) {
      prgBank = (data & 0x30) >> 4;
      chrBank = (data & 0x03) >> 0;
    }
  }

  auto readCHR(uint addr) -> uint8 {
    if(addr & 0x2000) {
      if(settings.mirror == 0) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
      return ppu.readCIRAM(addr & 0x07ff);
    }
    addr = (chrBank * 0x2000) + (addr & 0x1fff);
    return Board::readCHR(addr);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    if(addr & 0x2000) {
      if(settings.mirror == 0) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
      return ppu.writeCIRAM(addr & 0x07ff, data);
    }
    addr = (chrBank * 0x2000) + (addr & 0x1fff);
    Board::writeCHR(addr, data);
  }

  auto power() -> void {
    prgBank = 0;
    chrBank = 0;
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);
    s.integer(prgBank);
    s.integer(chrBank);
  }

  struct Settings {
    bool mirror;  //0 = horizontal, 1 = vertical
  } settings;

  uint2 prgBank;
  uint2 chrBank;
};
