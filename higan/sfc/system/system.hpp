struct System {
  enum class Region : uint { NTSC, PAL };

  inline auto loaded() const -> bool { return information.loaded; }
  inline auto region() const -> Region { return information.region; }
  inline auto cpuFrequency() const -> double { return information.cpuFrequency; }
  inline auto apuFrequency() const -> double { return information.apuFrequency; }

  auto run() -> void;
  auto runToSave() -> void;

  auto load(Emulator::Interface*) -> bool;
  auto save() -> void;
  auto unload() -> void;
  auto power(bool reset) -> void;

  //video.cpp
  auto configureVideoPalette() -> void;
  auto configureVideoEffects() -> void;

  //serialization.cpp
  auto serialize() -> serializer;
  auto unserialize(serializer&) -> bool;

private:
  Emulator::Interface* interface = nullptr;

  struct Information {
    string manifest;
    bool loaded = false;
    Region region = Region::NTSC;
    double cpuFrequency = Emulator::Constants::Colorburst::NTSC * 6.0;
    double apuFrequency = 32040.0 * 768.0;
  } information;

  uint serializeSize = 0;

  auto serialize(serializer&) -> void;
  auto serializeAll(serializer&) -> void;
  auto serializeInit() -> void;

  friend class Cartridge;
};

extern System system;

auto Region::NTSC() -> bool { return system.region() == System::Region::NTSC; }
auto Region::PAL() -> bool { return system.region() == System::Region::PAL; }
