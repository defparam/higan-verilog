namespace Heuristics {

struct GameBoy {
  GameBoy(vector<uint8_t>& data, string location);
  explicit operator bool() const;
  auto manifest() const -> string;

private:
  auto read(uint offset) const -> uint8_t { return data[headerAddress + offset]; }

  vector<uint8_t>& data;
  string location;
  uint headerAddress = 0;
};

GameBoy::GameBoy(vector<uint8_t>& data, string location) : data(data), location(location) {
  headerAddress = data.size() < 0x8000 ? data.size() : data.size() - 0x8000;
  if(read(0x0104) == 0xce && read(0x0105) == 0xed && read(0x0106) == 0x66 && read(0x0107) == 0x66
  && read(0x0108) == 0xcc && read(0x0109) == 0x0d && read(0x0147) >= 0x0b && read(0x0147) <= 0x0d
  ) {  //MMM01 stores header at bottom of data[]
  } else { //all other mappers store header at top of data[]
    headerAddress = 0;
  }
}

GameBoy::operator bool() const {
  return data.size() >= 0x4000;
}

auto GameBoy::manifest() const -> string {
  if(!operator bool()) return {};

  bool black = (read(0x0143) & 0xc0) == 0x80;  //cartridge works in DMG+CGB mode
  bool clear = (read(0x0143) & 0xc0) == 0xc0;  //cartridge works in CGB mode only

  bool flash = false;
  bool battery = false;
  bool ram = false;
  bool rtc = false;
  bool accelerometer = false;
  bool rumble = false;

  uint romSize = 0;
  uint ramSize = 0;
  uint flashSize = 0;
  uint rtcSize = 0;

  string mapper = "MBC0";

  switch(read(0x0147)) {

  case 0x00:
    mapper = "MBC0";
    break;

  case 0x01:
    mapper = "MBC1";
    break;

  case 0x02:
    mapper = "MBC1";
    ram = true;
    break;

  case 0x03:
    mapper = "MBC1";
    battery = true;
    ram = true;
    break;

  case 0x05:
    mapper = "MBC2";
    ram = true;
    break;

  case 0x06:
    mapper = "MBC2";
    battery = true;
    ram = true;
    break;

  case 0x08:
    mapper = "MBC0";
    ram = true;
    break;

  case 0x09:
    mapper = "MBC0";
    battery = true;
    ram = true;
    break;

  case 0x0b:
    mapper = "MMM01";
    break;

  case 0x0c:
    mapper = "MMM01";
    ram = true;
    break;

  case 0x0d:
    mapper = "MMM01";
    battery = true;
    ram = true;
    break;

  case 0x0f:
    mapper = "MBC3";
    battery = true;
    rtc = true;
    break;

  case 0x10:
    mapper = "MBC3";
    battery = true;
    ram = true;
    rtc = true;
    break;

  case 0x11:
    mapper = "MBC3";
    break;

  case 0x12:
    mapper = "MBC3";
    ram = true;
    break;

  case 0x13:
    mapper = "MBC3";
    battery = true;
    ram = true;
    break;

  case 0x19:
    mapper = "MBC5";
    break;

  case 0x1a:
    mapper = "MBC5";
    ram = true;
    break;

  case 0x1b:
    mapper = "MBC5";
    battery = true;
    ram = true;
    break;

  case 0x1c:
    mapper = "MBC5";
    rumble = true;
    break;

  case 0x1d:
    mapper = "MBC5";
    ram = true;
    rumble = true;
    break;

  case 0x1e:
    mapper = "MBC5";
    battery = true;
    ram = true;
    rumble = true;
    break;

  case 0x20:
    mapper = "MBC6";
    flash = true;
    battery = true;
    ram = true;
    break;

  case 0x22:
    mapper = "MBC7";
    battery = true;
    ram = true;
    accelerometer = true;
    rumble = true;
    break;

  case 0xfc:
    mapper = "CAMERA";
    break;

  case 0xfd:
    mapper = "TAMA";
    battery = true;
    ram = true;
    rtc = true;
    break;

  case 0xfe:
    mapper = "HuC3";
    break;

  case 0xff:
    mapper = "HuC1";
    battery = true;
    ram = true;
    break;

  }

  switch(read(0x0148)) { default:
  case 0x00: romSize =   2 * 16 * 1024; break;
  case 0x01: romSize =   4 * 16 * 1024; break;
  case 0x02: romSize =   8 * 16 * 1024; break;
  case 0x03: romSize =  16 * 16 * 1024; break;
  case 0x04: romSize =  32 * 16 * 1024; break;
  case 0x05: romSize =  64 * 16 * 1024; break;
  case 0x06: romSize = 128 * 16 * 1024; break;
  case 0x07: romSize = 256 * 16 * 1024; break;
  case 0x52: romSize =  72 * 16 * 1024; break;
  case 0x53: romSize =  80 * 16 * 1024; break;
  case 0x54: romSize =  96 * 16 * 1024; break;
  }

  switch(read(0x0149)) { default:
  case 0x00: ramSize =  0 * 1024; break;
  case 0x01: ramSize =  2 * 1024; break;
  case 0x02: ramSize =  8 * 1024; break;
  case 0x03: ramSize = 32 * 1024; break;
  }

  if(mapper == "MBC2" && ram) ramSize = 256;
  if(mapper == "MBC6" && ram) ramSize =  32 * 1024;
  if(mapper == "MBC7" && ram) ramSize = 256;
  if(mapper == "TAMA" && ram) ramSize =  32;

  if(mapper == "MBC6" && flash) flashSize = 1024 * 1024;

  if(mapper == "MBC3" && rtc) rtcSize = 13;
  if(mapper == "TAMA" && rtc) rtcSize = 21;

  string output;
  output.append("game\n");
  output.append("  sha256: ", Hash::SHA256(data).digest(), "\n");
  output.append("  label:  ", Location::prefix(location), "\n");
  output.append("  name:   ", Location::prefix(location), "\n");
  output.append("  board:  ", mapper, "\n");
  output.append(Memory{}.type("ROM").size(data.size()).category("Program").text());
if(ram && ramSize)
  output.append(Memory{}.type("RAM").size(ramSize).category("Save").battery(battery).text());
if(flash && flashSize)
  output.append(Memory{}.type("Flash").size(flashSize).category("Download").text());
if(rtc && rtcSize)
  output.append(Memory{}.type("RTC").size(rtcSize).category("Time").battery().text());
if(accelerometer)
  output.append("    accelerometer\n");
if(rumble)
  output.append("    rumble\n");
  return output;
}

}
