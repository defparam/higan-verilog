auto System::serialize() -> serializer {
  serializer s(_serializeSize);

  uint signature = 0x31545342;
  char version[16] = {0};
  char hash[64] = {0};
  char description[512] = {0};
  memory::copy(&version, (const char*)Emulator::SerializerVersion, Emulator::SerializerVersion.size());
  memory::copy(&hash, (const char*)cartridge.sha256(), 64);

  s.integer(signature);
  s.array(version);
  s.array(hash);
  s.array(description);

  serializeAll(s);
  return s;
}

auto System::unserialize(serializer& s) -> bool {
  uint signature;
  char version[16] = {0};
  char hash[64] = {0};
  char description[512];

  s.integer(signature);
  s.array(version);
  s.array(hash);
  s.array(description);

  if(signature != 0x31545342) return false;
  if(string{version} != Emulator::SerializerVersion) return false;

  power();
  serializeAll(s);
  return true;
}

auto System::serialize(serializer& s) -> void {
  s.integer(_clocksExecuted);
}

auto System::serializeAll(serializer& s) -> void {
  cartridge.serialize(s);
  system.serialize(s);
  cpu.serialize(s);
  ppu.serialize(s);
  apu.serialize(s);
}

auto System::serializeInit() -> void {
  serializer s;

  uint signature = 0;
  char version[16] = {0};
  char hash[64] = {0};
  char description[512] = {0};

  s.integer(signature);
  s.array(version);
  s.array(hash);
  s.array(description);

  serializeAll(s);
  _serializeSize = s.size();
}
