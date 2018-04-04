struct Cartridge {
  auto pathID() const -> uint { return information.pathID; }
  auto region() const -> string { return information.region; }
  auto sha256() const -> string { return information.sha256; }
  auto manifest() const -> string;
  auto title() const -> string;

  auto load() -> bool;
  auto save() -> void;
  auto unload() -> void;

  auto serialize(serializer&) -> void;

  MappedRAM rom;
  MappedRAM ram;

  struct Information {
    uint pathID = 0;
    string region;
    string sha256;
  } information;

  struct Has {
    boolean ICD;
    boolean MCC;
    boolean NSSDIP;
    boolean Event;
    boolean SA1;
    boolean SuperFX;
    boolean ARMDSP;
    boolean HitachiDSP;
    boolean NECDSP;
    boolean EpsonRTC;
    boolean SharpRTC;
    boolean SPC7110;
    boolean SDD1;
    boolean OBC1;
    boolean MSU1;

    boolean GameBoySlot;
    boolean BSMemorySlot;
    boolean SufamiTurboSlots;
  } has;

private:
  Emulator::Game game;
  Emulator::Game slotGameBoy;
  Emulator::Game slotBSMemory;
  Emulator::Game slotSufamiTurboA;
  Emulator::Game slotSufamiTurboB;
  Markup::Node board;

  //cartridge.cpp
  auto loadGameBoy() -> bool;
  auto loadBSMemory() -> bool;
  auto loadSufamiTurboA() -> bool;
  auto loadSufamiTurboB() -> bool;

  //load.cpp
  auto loadBoard(string) -> Markup::Node;
  auto loadCartridge(Markup::Node) -> void;
  auto loadGameBoy(Markup::Node) -> void;
  auto loadBSMemory(Markup::Node) -> void;
  auto loadSufamiTurboA(Markup::Node) -> void;
  auto loadSufamiTurboB(Markup::Node) -> void;

  auto loadROM(Markup::Node) -> void;
  auto loadRAM(Markup::Node) -> void;
  auto loadICD(Markup::Node) -> void;
  auto loadMCC(Markup::Node) -> void;
  auto loadBSMemoryPack(Markup::Node) -> void;
  auto loadSufamiTurbo(Markup::Node, bool slot) -> void;
  auto loadNSS(Markup::Node) -> void;
  auto loadEvent(Markup::Node) -> void;
  auto loadSA1(Markup::Node) -> void;
  auto loadSuperFX(Markup::Node) -> void;
  auto loadARMDSP(Markup::Node) -> void;
  auto loadHitachiDSP(Markup::Node, uint roms) -> void;
  auto loadNECDSP(Markup::Node) -> void;
  auto loadEpsonRTC(Markup::Node) -> void;
  auto loadSharpRTC(Markup::Node) -> void;
  auto loadSPC7110(Markup::Node) -> void;
  auto loadSDD1(Markup::Node) -> void;
  auto loadOBC1(Markup::Node) -> void;
  auto loadMSU1(Markup::Node) -> void;

  auto loadMemory(MappedRAM&, Markup::Node, bool required, maybe<uint> id = nothing) -> void;
  auto loadMap(Markup::Node, SuperFamicom::Memory&) -> void;
  auto loadMap(Markup::Node, const function<uint8 (uint24, uint8)>&, const function<void (uint24, uint8)>&) -> void;

  //save.cpp
  auto saveCartridge(Markup::Node) -> void;
  auto saveGameBoy(Markup::Node) -> void;
  auto saveBSMemory(Markup::Node) -> void;
  auto saveSufamiTurboA(Markup::Node) -> void;
  auto saveSufamiTurboB(Markup::Node) -> void;

  auto saveRAM(Markup::Node) -> void;
  auto saveMCC(Markup::Node) -> void;
  auto saveEvent(Markup::Node) -> void;
  auto saveSA1(Markup::Node) -> void;
  auto saveSuperFX(Markup::Node) -> void;
  auto saveARMDSP(Markup::Node) -> void;
  auto saveHitachiDSP(Markup::Node) -> void;
  auto saveNECDSP(Markup::Node) -> void;
  auto saveEpsonRTC(Markup::Node) -> void;
  auto saveSharpRTC(Markup::Node) -> void;
  auto saveSPC7110(Markup::Node) -> void;
  auto saveSDD1(Markup::Node) -> void;
  auto saveOBC1(Markup::Node) -> void;

  auto saveMemory(MappedRAM&, Markup::Node, maybe<uint> = nothing) -> void;

  friend class Interface;
  friend class ICD;
};

extern Cartridge cartridge;
