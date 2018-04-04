namespace Heuristics {

struct GameGear {
  GameGear(vector<uint8_t>& data, string location);
  explicit operator bool() const;
  auto manifest() const -> string;

private:
  vector<uint8_t>& data;
  string location;
};

GameGear::GameGear(vector<uint8_t>& data, string location) : data(data), location(location) {
}

GameGear::operator bool() const {
  return (bool)data;
}

auto GameGear::manifest() const -> string {
  string output;
  output.append("game\n");
  output.append("  sha256: ", Hash::SHA256(data).digest(), "\n");
  output.append("  label:  ", Location::prefix(location), "\n");
  output.append("  name:   ", Location::prefix(location), "\n");
  output.append("  board\n");
  output.append(Memory{}.type("ROM").size(data.size()).category("Program").text());
  output.append(Memory{}.type("RAM").size(0x8000).category("Save").battery().text());
  return output;
}

}
