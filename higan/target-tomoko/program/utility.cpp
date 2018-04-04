auto Program::initializeVideoDriver() -> void {
  if(!Video::availableDrivers().find(settings["Video/Driver"].text())) {
    settings["Video/Driver"].setValue("None");
  }
  video = Video::create(settings["Video/Driver"].text());
  video->setContext(presentation->viewport.handle());

  video->setBlocking(settings["Video/Synchronize"].boolean());

  if(!video->ready()) {
    MessageDialog().setText("Failed to initialize video driver").warning();
    video = Video::create("None");
  }

  presentation->clearViewport();
}

auto Program::initializeAudioDriver() -> void {
  if(!Audio::availableDrivers().find(settings["Audio/Driver"].text())) {
    settings["Audio/Driver"].setValue("None");
  }
  audio = Audio::create(settings["Audio/Driver"].text());
  audio->setContext(presentation->viewport.handle());

  if(!audio->availableDevices().find(settings["Audio/Device"].text())) {
    settings["Audio/Device"].setValue(audio->availableDevices()(0));
  }
  audio->setDevice(settings["Audio/Device"].text());

  if(!audio->availableFrequencies().find(settings["Audio/Frequency"].real())) {
    settings["Audio/Frequency"].setValue(audio->availableFrequencies()(0));
  }
  audio->setFrequency(settings["Audio/Frequency"].real());

  if(!audio->availableLatencies().find(settings["Audio/Latency"].natural())) {
    settings["Audio/Latency"].setValue(audio->availableLatencies()(0));
  }
  audio->setLatency(settings["Audio/Latency"].real());

  audio->setChannels(2);
  audio->setExclusive(settings["Audio/Exclusive"].boolean());
  audio->setBlocking(settings["Audio/Synchronize"].boolean());

  if(!audio->ready()) {
    MessageDialog().setText("Failed to initialize audio driver").warning();
    audio = Audio::create("None");
  }

  Emulator::audio.setFrequency(settings["Audio/Frequency"].real());
}

auto Program::initializeInputDriver() -> void {
  if(!Input::availableDrivers().find(settings["Input/Driver"].text())) {
    settings["Input/Driver"].setValue("None");
  }
  input = Input::create(settings["Input/Driver"].text());
  input->setContext(presentation->viewport.handle());

  input->onChange({&InputManager::onChange, &inputManager()});

  if(!input->ready()) {
    MessageDialog().setText("Failed to initialize input driver").warning();
    input = Input::create("None");
  }
}

auto Program::softReset() -> void {
  if(!emulator) return;
  if(!emulator->information.resettable) return;
  emulator->reset();
  showMessage("System has been soft reset");
}

auto Program::powerCycle() -> void {
  if(!emulator) return;
  emulator->power();
  showMessage("System has been power cycled");
}

auto Program::togglePause() -> void {
  program->pause = !program->pause;
  presentation->pauseEmulation.setChecked(program->pause);
  if(program->pause) {
    showMessage("Emulation has been paused");
  } else {
    showMessage("Emulation has been unpaused");
  }
}

auto Program::rotateDisplay() -> void {
  if(!emulator) return;
  if(!emulator->cap("Rotate Display")) return showMessage("Display rotation not supported");
  auto rotate = emulator->get("Rotate Display");
  emulator->set("Rotate Display", !rotate.get<bool>());
  presentation->resizeViewport();
  showMessage("Display rotated");
}

auto Program::connectDevices() -> void {
  if(!emulator) return;
  for(auto& port : emulator->ports) {
    auto path = string{emulator->information.name, "/", port.name}.replace(" ", "");
    auto name = settings(path).text();
    for(auto& device : port.devices) {
      if(device.name == name) {
        emulator->connect(port.id, device.id);
        break;
      }
    }
  }
}

auto Program::showMessage(const string& text) -> void {
  statusTime = time(nullptr);
  statusMessage = text;
}

auto Program::updateStatusText() -> void {
  time_t currentTime = time(nullptr);

  string text;
  if((currentTime - statusTime) <= 2) {
    text = statusMessage;
  } else if(!emulator || emulator->loaded() == false) {
    text = "No cartridge loaded";
  } else if(pause || (!presentation->focused() && settings["Input/FocusLoss/Pause"].boolean())) {
    text = "Paused";
  } else {
    text = statusText;
  }

  if(text != presentation->statusBar.text()) {
    presentation->statusBar.setText(text);
  }
}

auto Program::updateVideoPalette() -> void {
  double saturation = settings["Video/Saturation"].natural() / 100.0;
  double gamma = settings["Video/Gamma"].natural() / 100.0;
  double luminance = settings["Video/Luminance"].natural() / 100.0;
  Emulator::video.setSaturation(saturation);
  Emulator::video.setGamma(gamma);
  Emulator::video.setLuminance(luminance);
  Emulator::video.setPalette();
}

auto Program::updateVideoShader() -> void {
  if(settings["Video/Driver"].text() == "OpenGL"
  && settings["Video/Shader"].text() != "None"
  && settings["Video/Shader"].text() != "Blur"
  && directory::exists(settings["Video/Shader"].text())
  ) {
    video->setSmooth(false);
    video->setShader(settings["Video/Shader"].text());
  } else {
    video->setSmooth(settings["Video/Shader"].text() == "Blur");
    video->setShader("");
  }
}

auto Program::updateAudioDriver() -> void {
  if(!audio) return;
  audio->clear();
  audio->setExclusive(settings["Audio/Exclusive"].boolean());
  audio->setFrequency(settings["Audio/Frequency"].real());
  audio->setLatency(settings["Audio/Latency"].natural());
  Emulator::audio.setFrequency(settings["Audio/Frequency"].real());
}

auto Program::updateAudioEffects() -> void {
  auto volume = settings["Audio/Mute"].boolean() ? 0.0 : settings["Audio/Volume"].natural() * 0.01;
  Emulator::audio.setVolume(volume);

  auto balance = max(-1.0, min(1.0, (settings["Audio/Balance"].integer() - 50) / 50.0));
  Emulator::audio.setBalance(balance);

  auto reverbEnable = settings["Audio/Reverb/Enable"].boolean();
  Emulator::audio.setReverb(reverbEnable);
}

auto Program::focused() -> bool {
  //exclusive mode creates its own top-level window: presentation window will not have focus
  if(video->exclusive()) return true;
  if(presentation && presentation->focused()) return true;
  return false;
}
