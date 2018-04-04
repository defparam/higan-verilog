struct Program : Emulator::Platform {
  //program.cpp
  Program(string_vector args);
  auto main() -> void;
  auto quit() -> void;

  //interface.cpp
  auto path(uint id) -> string override;
  auto open(uint id, string name, vfs::file::mode mode, bool required) -> vfs::shared::file override;
  auto load(uint id, string name, string type, string_vector options = {}) -> Emulator::Platform::Load override;
  auto videoRefresh(const uint32* data, uint pitch, uint width, uint height) -> void override;
  auto audioSample(const double* samples, uint channels) -> void override;
  auto inputPoll(uint port, uint device, uint input) -> int16 override;
  auto inputRumble(uint port, uint device, uint input, bool enable) -> void override;
  auto dipSettings(Markup::Node node) -> uint override;
  auto notify(string text) -> void override;

  //medium.cpp
  auto loadMedium() -> void;
  auto loadMedium(Emulator::Interface& interface, const Emulator::Interface::Medium& medium) -> void;
  auto unloadMedium() -> void;

  //state.cpp
  auto stateName(uint slot, bool managed = false) -> string;
  auto loadState(uint slot, bool managed = false) -> bool;
  auto saveState(uint slot, bool managed = false) -> bool;

  //utility.cpp
  auto initializeVideoDriver() -> void;
  auto initializeAudioDriver() -> void;
  auto initializeInputDriver() -> void;

  auto softReset() -> void;
  auto powerCycle() -> void;
  auto togglePause() -> void;
  auto rotateDisplay() -> void;
  auto connectDevices() -> void;
  auto showMessage(const string& text) -> void;
  auto updateStatusText() -> void;
  auto updateVideoPalette() -> void;
  auto updateVideoShader() -> void;
  auto updateAudioDriver() -> void;
  auto updateAudioEffects() -> void;
  auto focused() -> bool;

  bool hasQuit = false;
  bool pause = false;

  vector<Emulator::Interface*> emulators;

  vector<string> mediumQueue;  //for command-line and drag-and-drop loading
  vector<string> mediumPaths;  //for keeping track of loaded folder locations

  time_t autoSaveTime = 0;  //for automatically saving RAM periodically

  string statusText;
  string statusMessage;
  time_t statusTime = 0;
};

extern unique_pointer<Program> program;
