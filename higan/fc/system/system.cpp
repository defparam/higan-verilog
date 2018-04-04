#include <fc/fc.hpp>

namespace Famicom {

#include "video.cpp"
#include "serialization.cpp"
System system;
Scheduler scheduler;
Cheat cheat;

auto System::run() -> void {
  if(scheduler.enter() == Scheduler::Event::Frame) ppu.refresh();
}

auto System::runToSave() -> void {
  scheduler.synchronize(cpu);
  scheduler.synchronize(apu);
  scheduler.synchronize(ppu);
  scheduler.synchronize(cartridge);
  for(auto peripheral : cpu.peripherals) scheduler.synchronize(*peripheral);
}

auto System::load(Emulator::Interface* interface) -> bool {
  information = {};

  if(auto fp = platform->open(ID::System, "manifest.bml", File::Read, File::Required)) {
    information.manifest = fp->reads();
  } else {
    return false;
  }
  auto document = BML::unserialize(information.manifest);
  if(!cartridge.load()) return false;

  if(cartridge.region() == "NTSC-J") {
    information.region = Region::NTSCJ;
    information.frequency = Emulator::Constants::Colorburst::NTSC * 6.0;
  }
  if(cartridge.region() == "NTSC-U") {
    information.region = Region::NTSCU;
    information.frequency = Emulator::Constants::Colorburst::NTSC * 6.0;
  }
  if(cartridge.region() == "PAL") {
    information.region = Region::PAL;
    information.frequency = Emulator::Constants::Colorburst::PAL * 6.0;
  }

  this->interface = interface;
  serializeInit();
  return information.loaded = true;
}

auto System::save() -> void {
  cartridge.save();
}

auto System::unload() -> void {
  if(!loaded()) return;
  cpu.peripherals.reset();
  controllerPort1.unload();
  controllerPort2.unload();
  cartridge.unload();
  information.loaded = false;
}

auto System::power(bool reset) -> void {
  Emulator::video.reset();
  Emulator::video.setInterface(interface);
  configureVideoPalette();
  configureVideoEffects();

  Emulator::audio.reset();
  Emulator::audio.setInterface(interface);

  scheduler.reset();
  cartridge.power();
  cpu.power(reset);
  apu.power(reset);
  ppu.power(reset);
  scheduler.primary(cpu);

  controllerPort1.power(ID::Port::Controller1);
  controllerPort2.power(ID::Port::Controller2);

  controllerPort1.connect(settings.controllerPort1);
  controllerPort2.connect(settings.controllerPort2);
}

auto System::init() -> void {
  assert(interface != nullptr);
}

auto System::term() -> void {
}

}
