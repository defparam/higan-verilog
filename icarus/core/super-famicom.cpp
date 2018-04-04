auto Icarus::superFamicomManifest(string location) -> string {
  vector<uint8_t> buffer;
  auto files = directory::files(location, "*.rom");
  concatenate(buffer, {location, "program.rom"});
  concatenate(buffer, {location, "data.rom"   });
  for(auto& file : files.match("*.program.rom")) concatenate(buffer, {location, file});
  for(auto& file : files.match("*.data.rom"   )) concatenate(buffer, {location, file});
  for(auto& file : files.match("*.boot.rom"   )) concatenate(buffer, {location, file});
  return superFamicomManifest(buffer, location);
}

auto Icarus::superFamicomManifest(vector<uint8_t>& buffer, string location) -> string {
  if(settings["icarus/UseDatabase"].boolean()) {
    auto digest = Hash::SHA256(buffer).digest();
    for(auto game : Database::SuperFamicom.find("game")) {
      if(game["sha256"].text() == digest) return BML::serialize(game);
    }
  }

  if(settings["icarus/UseHeuristics"].boolean()) {
    Heuristics::SuperFamicom game{buffer, location};
    if(auto manifest = game.manifest()) return manifest;
  }

  return {};
}

auto Icarus::superFamicomImport(vector<uint8_t>& buffer, string location) -> string {
  auto name = Location::prefix(location);
  auto source = Location::path(location);
  string target{settings["Library/Location"].text(), "Super Famicom/", name, ".sfc/"};

  auto manifest = superFamicomManifest(buffer, location);
  if(!manifest) return failure("failed to parse ROM image");

  if(!create(target)) return failure("library path unwritable");
  if(exists({source, name, ".srm"}) && !exists({target, "save.ram"})) {
    copy({source, name, ".srm"}, {target, "save.ram"});
  }

  if(settings["icarus/CreateManifests"].boolean()) write({target, "manifest.bml"}, manifest);
  uint offset = 0;
  auto document = BML::unserialize(manifest);
  for(auto rom : document.find("game/board/memory")) {
    if(rom["type"].text() != "ROM") continue;
    auto name = string{rom["part"].text(), ".", rom["category"].text(), ".rom"}.trimLeft(".", 1L).downcase();
    auto size = rom["size"].natural();
    if(size > buffer.size() - offset) {
      auto name = string{rom["note"].text(), ".", rom["category"].text(), ".rom"}.trimLeft(".", 1L).downcase();
      auto location = locate({"Firmware/", name});
      if(location && file::size(location) == size) {
        write({target, name}, file::read(location));
      } else {
        missingFiles.append(name);
      }
      continue;
    }
    write({target, name}, buffer.data() + offset, size);
    offset += size;
  }
  if(missingFiles) return failure({"ROM image is missing data: ", missingFiles.merge("; ")});
  return success(target);
}
