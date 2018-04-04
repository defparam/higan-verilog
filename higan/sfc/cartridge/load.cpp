auto Cartridge::loadBoard(string board) -> Markup::Node {
  string output;

  if(board.beginsWith("SNSP-")) board.replace("SNSP-", "SHVC-", 1L);
  if(board.beginsWith("MAXI-")) board.replace("MAXI-", "SHVC-", 1L);
  if(board.beginsWith("MJSC-")) board.replace("MJSC-", "SHVC-", 1L);
  if(board.beginsWith("EA-"  )) board.replace("EA-",   "SHVC-", 1L);

  if(auto fp = platform->open(ID::System, "boards.bml", File::Read, File::Required)) {
    auto document = BML::unserialize(fp->reads());
    for(auto leaf : document.find("board")) {
      auto id = leaf.text();
      bool matched = id == board;
      if(!matched && id.match("*(*)*")) {
        auto part = id.transform("()", "||").split("|");
        for(auto& revision : part(1).split(",")) {
          if(string{part(0), revision, part(2)} == board) matched = true;
        }
      }
      if(matched) return leaf;
    }
  }

  return {};
}

auto Cartridge::loadCartridge(Markup::Node node) -> void {
  board = loadBoard(game.board);

  if(region() == "Auto") {
    auto region = game.region;
    if(region.endsWith("BRA")
    || region.endsWith("CAN")
    || region.endsWith("HKG")
    || region.endsWith("JPN")
    || region.endsWith("KOR")
    || region.endsWith("LTN")
    || region.endsWith("ROC")
    || region.endsWith("USA")
    || region.beginsWith("SHVC-")
    || region == "NTSC") {
      information.region = "NTSC";
    } else {
      information.region = "PAL";
    }
  }

  if(board["bsmemory"] || board["mcc/bsmemory"] || board["sa1/bsmemory"]) {
    if(auto loaded = platform->load(ID::BSMemory, "BS Memory", "bs")) {
      bsmemory.pathID = loaded.pathID();
      loadBSMemory();
    }
  }

  if(board["sufamiturbo"]) {
    if(auto loaded = platform->load(ID::SufamiTurboA, "Sufami Turbo", "st")) {
      sufamiturboA.pathID = loaded.pathID();
      loadSufamiTurboA();
    }
  }

  if(auto node = board["memory(type=ROM)"]) loadROM(node);
  if(auto node = board["memory(type=RAM)"]) loadRAM(node);
  if(auto node = board["icd"]) loadICD(node);
  if(auto node = board["mcc"]) loadMCC(node);
  if(auto node = board["bsmemory"]) loadBSMemoryPack(node);
  if(auto node = board.find("sufamiturbo")) if(node(0)) loadSufamiTurbo(node(0), 0);
  if(auto node = board.find("sufamiturbo")) if(node(1)) loadSufamiTurbo(node(1), 1);
  if(auto node = board["nss"]) loadNSS(node);
  if(auto node = board["event"]) loadEvent(node);
  if(auto node = board["sa1"]) loadSA1(node);
  if(auto node = board["superfx"]) loadSuperFX(node);
  if(auto node = board["armdsp"]) loadARMDSP(node);
  if(auto node = board["hitachidsp"]) loadHitachiDSP(node, node["information/board"].text().match("2DC*") ? 2 : 1);
  if(auto node = board["necdsp"]) loadNECDSP(node);
  if(auto node = board["epsonrtc"]) loadEpsonRTC(node);
  if(auto node = board["sharprtc"]) loadSharpRTC(node);
  if(auto node = board["spc7110"]) loadSPC7110(node);
  if(auto node = board["sdd1"]) loadSDD1(node);
  if(auto node = board["obc1"]) loadOBC1(node);
  if(auto node = board["msu1"]) loadMSU1(node);
}

auto Cartridge::loadGameBoy(Markup::Node node) -> void {
}

auto Cartridge::loadBSMemory(Markup::Node node) -> void {
  if(node["game/board/memory/type"].text() == "ROM") {
    bsmemory.readonly = true;
    loadMemory(bsmemory.memory, node["game/board/memory(type=ROM)"], File::Required, bsmemory.pathID);
  } else {
    bsmemory.readonly = false;
    loadMemory(bsmemory.memory, node["game/board/memory(type=Flash)"], File::Required, bsmemory.pathID);
  }
}

auto Cartridge::loadSufamiTurboA(Markup::Node node) -> void {
  loadMemory(sufamiturboA.rom, node["game/board/memory(type=ROM)"], File::Required, sufamiturboA.pathID);
  loadMemory(sufamiturboA.ram, node["game/board/memory(type=RAM)"], File::Optional, sufamiturboA.pathID);

  if(auto loaded = platform->load(ID::SufamiTurboB, "Sufami Turbo", "st")) {
    sufamiturboB.pathID = loaded.pathID();
    loadSufamiTurboB();
  }
}

auto Cartridge::loadSufamiTurboB(Markup::Node node) -> void {
  loadMemory(sufamiturboB.rom, node["game/board/memory(type=ROM)"], File::Required, sufamiturboB.pathID);
  loadMemory(sufamiturboB.ram, node["game/board/memory(type=RAM)"], File::Optional, sufamiturboB.pathID);
}

//

auto Cartridge::loadROM(Markup::Node node) -> void {
  loadMemory(rom, node, File::Required);
  for(auto leaf : node.find("map")) loadMap(leaf, rom);
}

auto Cartridge::loadRAM(Markup::Node node) -> void {
  loadMemory(ram, node, File::Optional);
  for(auto leaf : node.find("map")) loadMap(leaf, ram);
}

auto Cartridge::loadICD(Markup::Node node) -> void {
  has.GameBoySlot = true;
  has.ICD = true;
  icd.Revision = node["revision"].natural();
  icd.Frequency = node["frequency"].natural();

  //Game Boy core loads data through ICD interface
  for(auto leaf : node.find("map")) loadMap(leaf, {&ICD::readIO, &icd}, {&ICD::writeIO, &icd});
}

auto Cartridge::loadMCC(Markup::Node node) -> void {
  has.BSMemorySlot = true;
  has.MCC = true;

  loadMemory(mcc.rom, node["rom"], File::Required);
  loadMemory(mcc.ram, node["ram"], File::Optional);

  for(auto leaf : node.find("map")) leaf.text() == "mcu"
  ? loadMap(leaf, {&MCC::mcuRead, &mcc}, {&MCC::mcuWrite, &mcc})
  : loadMap(leaf, {&MCC::read, &mcc}, {&MCC::write, &mcc});
  for(auto leaf : node["ram"].find("map")) loadMap(leaf, mcc.ram);
}

auto Cartridge::loadBSMemoryPack(Markup::Node node) -> void {
  has.BSMemorySlot = true;

  for(auto leaf : node.find("map")) {
    if(bsmemory.memory.size() == 0) continue;
    loadMap(leaf, bsmemory);
  }
}

auto Cartridge::loadSufamiTurbo(Markup::Node node, bool slot) -> void {
  has.SufamiTurboSlots = true;

  for(auto leaf : node["rom"].find("map")) {
    auto& cart = (slot == 0 ? sufamiturboA : sufamiturboB);
    if(cart.rom.size() == 0) continue;
    loadMap(leaf, cart.rom);
  }

  for(auto leaf : node["ram"].find("map")) {
    auto& cart = (slot == 0 ? sufamiturboA : sufamiturboB);
    if(cart.ram.size() == 0) continue;
    loadMap(leaf, cart.ram);
  }
}

auto Cartridge::loadNSS(Markup::Node node) -> void {
  has.NSSDIP = true;
  nss.dip = platform->dipSettings(node);

  for(auto leaf : node.find("map")) loadMap(leaf, {&NSS::read, &nss}, {&NSS::write, &nss});
}

auto Cartridge::loadEvent(Markup::Node node) -> void {
  auto roms = node.find("rom");
  if(roms.size() != 4) return;

  has.Event = true;

  for(uint n : range(4)) loadMemory(event.rom[n], roms[n], File::Required);
  loadMemory(event.ram, node["ram"], File::Optional);

  event.board = Event::Board::CampusChallenge92;
  if(node.text() == "CC92") event.board = Event::Board::CampusChallenge92;
  if(node.text() == "PF94") event.board = Event::Board::Powerfest94;
  event.timer = node["timer"].natural();

  for(auto leaf : node.find("map")) leaf.text() == "mcu"
  ? loadMap(leaf, {&Event::mcuRead, &event}, {&Event::mcuWrite, &event})
  : loadMap(leaf, {&Event::read, &event}, {&Event::write, &event});
  for(auto leaf : node["ram"].find("map")) loadMap(leaf, event.ram);
}

auto Cartridge::loadSA1(Markup::Node node) -> void {
  has.SA1 = true;

  loadMemory(sa1.rom, node["rom"], File::Required);
  loadMemory(sa1.bwram, node["bwram"], File::Optional);
  loadMemory(sa1.iram, node["iram"], File::Optional);

  for(auto leaf : node.find("map")) loadMap(leaf, {&SA1::readIO, &sa1}, {&SA1::writeIO, &sa1});
  for(auto leaf : node["rom"].find("map")) loadMap(leaf, {&SA1::mmcromRead, &sa1}, {&SA1::mmcromWrite, &sa1});
  for(auto leaf : node["bwram"].find("map")) loadMap(leaf, {&SA1::mmcbwramRead, &sa1}, {&SA1::mmcbwramWrite, &sa1});
  for(auto leaf : node["iram"].find("map")) loadMap(leaf, sa1.cpuiram);
}

auto Cartridge::loadSuperFX(Markup::Node node) -> void {
  has.SuperFX = true;

  loadMemory(superfx.rom, node["rom"], File::Required);
  loadMemory(superfx.ram, node["ram"], File::Optional);

  for(auto leaf : node.find("map")) loadMap(leaf, {&SuperFX::readIO, &superfx}, {&SuperFX::writeIO, &superfx});
  for(auto leaf : node["rom"].find("map")) loadMap(leaf, superfx.cpurom);
  for(auto leaf : node["ram"].find("map")) loadMap(leaf, superfx.cpuram);
}

auto Cartridge::loadARMDSP(Markup::Node node) -> void {
  has.ARMDSP = true;

  if(auto memory = game.memory(node["memory(type=ROM,category=Program)"])) {
    if(auto fp = platform->open(ID::SuperFamicom, memory->name(), File::Read, File::Required)) {
      for(auto n : range(128 * 1024)) armdsp.programROM[n] = fp->read();
    }
  }
  if(auto memory = game.memory(node["memory(type=ROM,category=Data)"])) {
    if(auto fp = platform->open(ID::SuperFamicom, memory->name(), File::Read, File::Required)) {
      for(auto n : range( 32 * 1024)) armdsp.dataROM[n] = fp->read();
    }
  }
  if(auto memory = game.memory(node["memory(type=RAM,category=Data)"])) {
    if(auto fp = platform->open(ID::SuperFamicom, memory->name(), File::Read)) {
      for(auto n : range( 16 * 1024)) armdsp.programRAM[n] = fp->read();
    }
  }

  for(auto leaf : node.find("map")) loadMap(leaf, {&ArmDSP::read, &armdsp}, {&ArmDSP::write, &armdsp});
}

auto Cartridge::loadHitachiDSP(Markup::Node node, uint roms) -> void {
  has.HitachiDSP = true;

  hitachidsp.Frequency = node["frequency"].natural();
  if(hitachidsp.Frequency == 0) hitachidsp.Frequency = 20'000'000;
  hitachidsp.Roms = roms;  //1 or 2

  loadMemory(hitachidsp.rom, node["rom"], File::Required);
  loadMemory(hitachidsp.ram, node["ram"], File::Optional);

  for(auto& word : hitachidsp.dataROM) word = 0x000000;
  for(auto& word : hitachidsp.dataRAM) word = 0x00;

  if(auto memory = game.memory(node["memory(type=ROM,category=Data)"])) {
    if(auto fp = platform->open(ID::SuperFamicom, memory->name(), File::Read, File::Required)) {
      for(auto n : range(1 * 1024)) hitachidsp.dataROM[n] = fp->readl(3);
    }
  }
  if(auto memory = game.memory(node["memory(type=RAM,category=Data)"])) {
    if(auto fp = platform->open(ID::SuperFamicom, memory->name(), File::Read)) {
      for(auto n : range(3 * 1024)) hitachidsp.dataRAM[n] = fp->readl(1);
    }
  }

  for(auto leaf : node.find("map")) loadMap(leaf, {&HitachiDSP::dspRead, &hitachidsp}, {&HitachiDSP::dspWrite, &hitachidsp});
  for(auto leaf : node["rom"].find("map")) loadMap(leaf, {&HitachiDSP::romRead, &hitachidsp}, {&HitachiDSP::romWrite, &hitachidsp});
  for(auto leaf : node["ram"].find("map")) loadMap(leaf, {&HitachiDSP::ramRead, &hitachidsp}, {&HitachiDSP::ramWrite, &hitachidsp});
  for(auto leaf : node["dram"].find("map")) loadMap(leaf, {&HitachiDSP::dramRead, &hitachidsp}, {&HitachiDSP::dramWrite, &hitachidsp});
}

auto Cartridge::loadNECDSP(Markup::Node node) -> void {
  has.NECDSP = true;

  necdsp.Frequency = node["frequency"].natural();
  if(necdsp.Frequency == 0) necdsp.Frequency = 8000000;
  necdsp.revision
  = node["model"].text() == "uPD7725"  ? NECDSP::Revision::uPD7725
  : node["model"].text() == "uPD96050" ? NECDSP::Revision::uPD96050
  : NECDSP::Revision::uPD7725;

  for(auto& word : necdsp.programROM) word = 0x000000;
  for(auto& word : necdsp.dataROM) word = 0x0000;
  for(auto& word : necdsp.dataRAM) word = 0x0000;

  uint size[3] = {0};
  if(necdsp.revision == NECDSP::Revision::uPD7725 ) memory::assign(size,  2048, 1024,  256);
  if(necdsp.revision == NECDSP::Revision::uPD96050) memory::assign(size, 16384, 2048, 2048);

  if(auto memory = game.memory(node["memory(type=ROM,category=Program)"])) {
    if(auto fp = platform->open(ID::SuperFamicom, memory->name(), File::Read, File::Required)) {
      for(auto n : range(size[0])) necdsp.programROM[n] = fp->readl(3);
    }
  }
  if(auto memory = game.memory(node["memory(type=ROM,category=Data)"])) {
    if(auto fp = platform->open(ID::SuperFamicom, memory->name(), File::Read, File::Required)) {
      for(auto n : range(size[1])) necdsp.dataROM[n] = fp->readl(2);
    }
  }
  if(auto memory = game.memory(node["memory(type=RAM,category=Data"])) {
    if(auto fp = platform->open(ID::SuperFamicom, memory->name(), File::Read)) {
      for(auto n : range(size[2])) necdsp.dataRAM[n] = fp->readl(2);
    }
  }

  for(auto leaf : node.find("map")) loadMap(leaf, {&NECDSP::read, &necdsp}, {&NECDSP::write, &necdsp});
  for(auto leaf : node["dram"].find("map")) loadMap(leaf, {&NECDSP::readRAM, &necdsp}, {&NECDSP::writeRAM, &necdsp});
}

auto Cartridge::loadEpsonRTC(Markup::Node node) -> void {
  has.EpsonRTC = true;

  epsonrtc.initialize();
  if(auto memory = game.memory(node["memory(type=RTC)"])) {
    if(auto fp = platform->open(ID::SuperFamicom, memory->name(), File::Read)) {
      uint8 data[16] = {0};
      for(auto& byte : data) byte = fp->read();
      epsonrtc.load(data);
    }
  }

  for(auto leaf : node.find("map")) loadMap(leaf, {&EpsonRTC::read, &epsonrtc}, {&EpsonRTC::write, &epsonrtc});
}

auto Cartridge::loadSharpRTC(Markup::Node node) -> void {
  has.SharpRTC = true;

  sharprtc.initialize();
  if(auto memory = game.memory(node["memory(type=RTC)"])) {
    if(auto fp = platform->open(ID::SuperFamicom, memory->name(), File::Read)) {
      uint8 data[16] = {0};
      for(auto& byte : data) byte = fp->read();
      sharprtc.load(data);
    }
  }

  for(auto leaf : node.find("map")) loadMap(leaf, {&SharpRTC::read, &sharprtc}, {&SharpRTC::write, &sharprtc});
}

auto Cartridge::loadSPC7110(Markup::Node node) -> void {
  has.SPC7110 = true;

  loadMemory(spc7110.prom, node["prom"], File::Required);
  loadMemory(spc7110.drom, node["drom"], File::Required);
  loadMemory(spc7110.ram, node["ram"], File::Optional);

  for(auto leaf : node.find("map")) leaf.text() == "mcu"
  ? loadMap(leaf, {&SPC7110::mcuromRead, &spc7110}, {&SPC7110::mcuromWrite, &spc7110})
  : loadMap(leaf, {&SPC7110::read, &spc7110}, {&SPC7110::write, &spc7110});
  for(auto leaf : node["ram"].find("map")) loadMap(leaf, {&SPC7110::mcuramRead, &spc7110}, {&SPC7110::mcuramWrite, &spc7110});
}

auto Cartridge::loadSDD1(Markup::Node node) -> void {
  has.SDD1 = true;

  loadMemory(sdd1.rom, node["rom"], File::Required);
  loadMemory(sdd1.ram, node["ram"], File::Optional);

  for(auto leaf : node.find("map")) loadMap(leaf, {&SDD1::read, &sdd1}, {&SDD1::write, &sdd1});
  for(auto leaf : node["rom"].find("map")) loadMap(leaf, {&SDD1::mcuromRead, &sdd1}, {&SDD1::mcuromWrite, &sdd1});
  for(auto leaf : node["ram"].find("map")) loadMap(leaf, {&SDD1::mcuramRead, &sdd1}, {&SDD1::mcuramWrite, &sdd1});
}

auto Cartridge::loadOBC1(Markup::Node node) -> void {
  has.OBC1 = true;

  loadMemory(obc1.ram, node["ram"], File::Optional);

  for(auto leaf : node.find("map")) loadMap(leaf, {&OBC1::read, &obc1}, {&OBC1::write, &obc1});
}

auto Cartridge::loadMSU1(Markup::Node node) -> void {
  has.MSU1 = true;

  for(auto leaf : node.find("map")) loadMap(leaf, {&MSU1::readIO, &msu1}, {&MSU1::writeIO, &msu1});
}

//

auto Cartridge::loadMemory(MappedRAM& ram, Markup::Node node, bool required, maybe<uint> id) -> void {
  if(!id) id = pathID();
  if(auto memory = game.memory(node)) {
    ram.allocate(memory->size);
    if(memory->type == "RAM" && !memory->battery) return;
    if(memory->type == "RTC" && !memory->battery) return;
    if(auto fp = platform->open(id(), memory->name(), File::Read, required)) {
      fp->read(ram.data(), ram.size());
    }
  }
}

auto Cartridge::loadMap(Markup::Node map, SuperFamicom::Memory& memory) -> void {
  auto addr = map["address"].text();
  auto size = map["size"].natural();
  auto base = map["base"].natural();
  auto mask = map["mask"].natural();
  if(size == 0) size = memory.size();
  if(size == 0) return;
  bus.map({&SuperFamicom::Memory::read, &memory}, {&SuperFamicom::Memory::write, &memory}, addr, size, base, mask);
}

auto Cartridge::loadMap(
  Markup::Node map,
  const function<uint8 (uint24, uint8)>& reader,
  const function<void (uint24, uint8)>& writer
) -> void {
  auto addr = map["address"].text();
  auto size = map["size"].natural();
  auto base = map["base"].natural();
  auto mask = map["mask"].natural();
  bus.map(reader, writer, addr, size, base, mask);
}
