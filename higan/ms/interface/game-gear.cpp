GameGearInterface::GameGearInterface() {
  information.manufacturer = "Sega";
  information.name         = "Game Gear";
  information.overscan     = false;

  media.append({ID::GameGear, "Game Gear", "gg"});

  Port hardware{ID::Port::Hardware, "Hardware"};

  { Device device{ID::Device::GameGearControls, "Controls"};
    device.inputs.append({0, "Up"});
    device.inputs.append({0, "Down"});
    device.inputs.append({0, "Left"});
    device.inputs.append({0, "Right"});
    device.inputs.append({0, "1"});
    device.inputs.append({0, "2"});
    device.inputs.append({0, "Start"});
    hardware.devices.append(device);
  }

  ports.append(move(hardware));
}

auto GameGearInterface::videoInformation() -> VideoInformation {
  VideoInformation vi;
  vi.width  = 160;
  vi.height = 144;
  vi.internalWidth  = 160;
  vi.internalHeight = 144;
  vi.aspectCorrection = 1.0;
  vi.refreshRate = (system.colorburst() * 15.0 / 5.0) / (262.0 * 684.0);
  return vi;
}

auto GameGearInterface::videoColors() -> uint32 {
  return 1 << 12;
}

auto GameGearInterface::videoColor(uint32 color) -> uint64 {
  uint4 B = color >> 8;
  uint4 G = color >> 4;
  uint4 R = color >> 0;

  uint64 r = image::normalize(R, 4, 16);
  uint64 g = image::normalize(G, 4, 16);
  uint64 b = image::normalize(B, 4, 16);

  return r << 32 | g << 16 | b << 0;
}

auto GameGearInterface::load(uint id) -> bool {
  if(id == ID::GameGear) return system.load(this, System::Model::GameGear);
  return false;
}
