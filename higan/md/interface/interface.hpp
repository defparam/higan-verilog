namespace MegaDrive {

struct ID {
  enum : uint {
    System,
    MegaDrive,
  };

  struct Port { enum : uint {
    Controller1,
    Controller2,
    Extension,
  };};

  struct Device { enum : uint {
    None,
    ControlPad,
    FightingPad,
  };};
};

struct Interface : Emulator::Interface {
  using Emulator::Interface::load;

  Interface();

  auto manifest() -> string override;
  auto title() -> string override;

  auto videoInformation() -> VideoInformation override;
  auto videoColors() -> uint32 override;
  auto videoColor(uint32 color) -> uint64 override;

  auto loaded() -> bool override;
  auto load(uint id) -> bool override;
  auto save() -> void override;
  auto unload() -> void override;

  auto connect(uint port, uint device) -> void override;
  auto power() -> void override;
  auto reset() -> void override;
  auto run() -> void override;

  auto serialize() -> serializer override;
  auto unserialize(serializer&) -> bool override;

  auto cheatSet(const string_vector& list) -> void override;

  auto cap(const string& name) -> bool override;
  auto get(const string& name) -> any override;
  auto set(const string& name, const any& value) -> bool override;
};

struct Settings {
  uint controllerPort1 = 0;
  uint controllerPort2 = 0;
  uint extensionPort = 0;
};

extern Settings settings;

}
