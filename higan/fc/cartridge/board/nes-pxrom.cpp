//MMC2

struct NES_PxROM : Board {
  NES_PxROM(Markup::Node& document) : Board(document) {
    revision = Revision::PNROM;
  }

  auto readPRG(uint addr) -> uint8 {
    if(addr < 0x6000) return cpu.mdr();
    if(addr < 0x8000) return prgram.read(addr);
    uint bank = 0;
    switch((addr / 0x2000) & 3) {
    case 0: bank = prgBank; break;
    case 1: bank = 0x0d; break;
    case 2: bank = 0x0e; break;
    case 3: bank = 0x0f; break;
    }
    return prgrom.read((bank * 0x2000) | (addr & 0x1fff));
  }

  auto writePRG(uint addr, uint8 data) -> void {
    if(addr < 0x6000) return;
    if(addr < 0x8000) return prgram.write(addr, data);

    switch(addr & 0xf000) {
    case 0xa000: prgBank = data & 0x0f; break;
    case 0xb000: chrBank[0][0] = data & 0x1f; break;
    case 0xc000: chrBank[0][1] = data & 0x1f; break;
    case 0xd000: chrBank[1][0] = data & 0x1f; break;
    case 0xe000: chrBank[1][1] = data & 0x1f; break;
    case 0xf000: mirror = data & 0x01; break;
    }
  }

  auto addrCIRAM(uint addr) const -> uint {
    switch(mirror) {
    case 0: return ((addr & 0x0400) >> 0) | (addr & 0x03ff);  //vertical mirroring
    case 1: return ((addr & 0x0800) >> 1) | (addr & 0x03ff);  //horizontal mirroring
    }
  }

  auto readCHR(uint addr) -> uint8 {
    if(addr & 0x2000) return ppu.readCIRAM(addrCIRAM(addr));
    bool region = addr & 0x1000;
    uint bank = chrBank[region][latch[region]];
    if((addr & 0x0ff8) == 0x0fd8) latch[region] = 0;
    if((addr & 0x0ff8) == 0x0fe8) latch[region] = 1;
    return Board::readCHR((bank * 0x1000) | (addr & 0x0fff));
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    if(addr & 0x2000) return ppu.writeCIRAM(addrCIRAM(addr), data);
    bool region = addr & 0x1000;
    uint bank = chrBank[region][latch[region]];
    if((addr & 0x0ff8) == 0x0fd8) latch[region] = 0;
    if((addr & 0x0ff8) == 0x0fe8) latch[region] = 1;
    return Board::writeCHR((bank * 0x1000) | (addr & 0x0fff), data);
  }

  auto power() -> void {
    prgBank = 0;
    chrBank[0][0] = 0;
    chrBank[0][1] = 0;
    chrBank[1][0] = 0;
    chrBank[1][1] = 0;
    mirror = 0;
    latch[0] = 0;
    latch[1] = 0;
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);

    s.integer(prgBank);
    s.integer(chrBank[0][0]);
    s.integer(chrBank[0][1]);
    s.integer(chrBank[1][0]);
    s.integer(chrBank[1][1]);
    s.integer(mirror);
    s.array(latch);
  }

  enum Revision : uint {
    PEEOROM,
    PNROM,
  } revision;

  uint4 prgBank;
  uint5 chrBank[2][2];
  bool mirror;
  bool latch[2];
};
