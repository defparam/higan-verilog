#if defined(Hiro_Application)

Application::State Application::state;

auto Application::doMain() -> void {
  if(state.onMain) return state.onMain();
}

auto Application::font() -> Font {
  return state.font;
}

auto Application::name() -> string {
  return state.name;
}

auto Application::onMain(const function<void ()>& callback) -> void {
  state.onMain = callback;
}

auto Application::run() -> void {
  return pApplication::run();
}

auto Application::pendingEvents() -> bool {
  return pApplication::pendingEvents();
}

auto Application::processEvents() -> void {
  return pApplication::processEvents();
}

auto Application::quit() -> void {
  state.quit = true;
  return pApplication::quit();
}

auto Application::scale() -> float {
  return state.scale;
}

auto Application::scale(float value) -> float {
  return value * state.scale;
}

auto Application::setFont(const Font& font) -> void {
  state.font = font;
}

auto Application::setName(const string& name) -> void {
  state.name = name;
}

auto Application::setScale(float scale) -> void {
  state.scale = scale;
}

auto Application::unscale(float value) -> float {
  return value * (1.0 / state.scale);
}

//Windows
//=======

auto Application::Windows::doModalChange(bool modal) -> void {
  if(state.windows.onModalChange) return state.windows.onModalChange(modal);
}

auto Application::Windows::onModalChange(const function<void (bool)>& callback) -> void {
  state.windows.onModalChange = callback;
}

//Cocoa
//=====

auto Application::Cocoa::doAbout() -> void {
  if(state.cocoa.onAbout) return state.cocoa.onAbout();
}

auto Application::Cocoa::doActivate() -> void {
  if(state.cocoa.onActivate) return state.cocoa.onActivate();
}

auto Application::Cocoa::doPreferences() -> void {
  if(state.cocoa.onPreferences) return state.cocoa.onPreferences();
}

auto Application::Cocoa::doQuit() -> void {
  if(state.cocoa.onQuit) return state.cocoa.onQuit();
}

auto Application::Cocoa::onAbout(const function<void ()>& callback) -> void {
  state.cocoa.onAbout = callback;
}

auto Application::Cocoa::onActivate(const function<void ()>& callback) -> void {
  state.cocoa.onActivate = callback;
}

auto Application::Cocoa::onPreferences(const function<void ()>& callback) -> void {
  state.cocoa.onPreferences = callback;
}

auto Application::Cocoa::onQuit(const function<void ()>& callback) -> void {
  state.cocoa.onQuit = callback;
}

//Internal
//========

auto Application::initialize() -> void {
  static bool initialized = false;
  if(initialized == false) {
    initialized = true;
    return pApplication::initialize();
  }
}

#endif
