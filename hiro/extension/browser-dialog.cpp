#if defined(Hiro_BrowserDialog)

struct BrowserDialogWindow {
  BrowserDialogWindow(BrowserDialog::State& state) : state(state) {}
  auto accept() -> void;
  auto activate() -> void;
  auto change() -> void;
  auto isFolder(const string& name) -> bool;
  auto isMatch(const string& name) -> bool;
  auto run() -> BrowserDialog::Response;
  auto setPath(string path) -> void;

private:
  Window window;
    VerticalLayout layout{&window};
      HorizontalLayout pathLayout{&layout, Size{~0, 0}, 5};
        LineEdit pathName{&pathLayout, Size{~0, 0}, 0};
        Button pathRefresh{&pathLayout, Size{0, 0}, 0};
        Button pathHome{&pathLayout, Size{0, 0}, 0};
        Button pathUp{&pathLayout, Size{0, 0}, 0};
      ListView view{&layout, Size{~0, ~0}, 5};
      HorizontalLayout controlLayout{&layout, Size{~0, 0}};
        ComboButton filterList{&controlLayout, Size{0, 0}, 5};
        LineEdit fileName{&controlLayout, Size{~0, 0}, 5};
        ComboButton optionList{&controlLayout, Size{0, 0}, 5};
        Button acceptButton{&controlLayout, Size{80, 0}, 5};
        Button cancelButton{&controlLayout, Size{80, 0}, 5};

  BrowserDialog::State& state;
  BrowserDialog::Response response;
  vector<string_vector> filters;
};

//accept button clicked, or enter pressed on file name line edit
//also called by table view activate after special case handling
auto BrowserDialogWindow::accept() -> void {
  auto batched = view.batched();

  if(state.action == "openFile" && batched) {
    string name = batched.left()->cell(0)->text();
    if(isFolder(name)) return setPath({state.path, name});
    response.selected.append(string{state.path, name});
  }

  if(state.action == "openFiles") {
    for(auto item : batched) {
      string name = item->cell(0)->text();
      response.selected.append(string{state.path, name, isFolder(name) ? "/" : ""});
    }
  }

  if(state.action == "openFolder" && batched) {
    string name = batched.left()->cell(0)->text();
    if(!isMatch(name)) return setPath({state.path, name});
    response.selected.append(string{state.path, name, "/"});
  }

  if(state.action == "saveFile") {
    string name = fileName.text();
    if(!name && batched) name = batched.left()->cell(0)->text();
    if(!name || isFolder(name)) return;
    if(file::exists({state.path, name})) {
      if(MessageDialog("File already exists. Overwrite it?").question() != "Yes") return;
    }
    response.selected.append(string{state.path, name});
  }

  if(state.action == "selectFolder" && batched) {
    string name = batched.left()->cell(0)->text();
    if(isFolder(name)) response.selected.append(string{state.path, name, "/"});
  }

  if(response.selected) window.setModal(false);
}

//table view item double-clicked, or enter pressed on selected table view item
auto BrowserDialogWindow::activate() -> void {
  auto selectedItem = view.selected();

  if(state.action == "saveFile" && selectedItem) {
    string name = selectedItem->cell(0)->text();
    if(isFolder(name)) return setPath({state.path, name});
    fileName.setText(name);
  }

  if(state.action == "selectFolder" && selectedItem) {
    string name = selectedItem->cell(0)->text();
    if(isFolder(name)) return setPath({state.path, name});
  }

  accept();
}

//table view item changed
auto BrowserDialogWindow::change() -> void {
  fileName.setText("");
  if(state.action == "saveFile") {
    if(auto selectedItem = view.selected()) {
      string name = selectedItem->cell(0)->text();
      if(!isFolder(name)) fileName.setText(name);
    }
  }
}

auto BrowserDialogWindow::isFolder(const string& name) -> bool {
  return directory::exists({state.path, name});
}

auto BrowserDialogWindow::isMatch(const string& name) -> bool {
  if(auto selectedItem = filterList.selected()) {
    for(auto& filter : filters[selectedItem->offset()]) {
      if(name.match(filter)) return true;
    }
  }
  return false;
}

auto BrowserDialogWindow::run() -> BrowserDialog::Response {
  response = {};

  layout.setMargin(5);
  pathName.onActivate([&] { setPath(pathName.text()); });
  pathRefresh.setBordered(false).setIcon(Icon::Action::Refresh).onActivate([&] { setPath(state.path); });
  pathHome.setBordered(false).setIcon(Icon::Go::Home).onActivate([&] { setPath(Path::user()); });
  pathUp.setBordered(false).setIcon(Icon::Go::Up).onActivate([&] { setPath(Location::dir(state.path)); });
  view.setBatchable(state.action == "openFiles").onActivate([&] { activate(); }).onChange([&] { change(); });
  filterList.setVisible(state.action != "selectFolder").onChange([&] { setPath(state.path); });
  for(auto& filter : state.filters) {
    auto part = filter.split("|", 1L);
    filterList.append(ComboButtonItem().setText(part.left()));
  }
  optionList.setVisible((bool)state.options).onChange([&] { response.option = optionList.selected().text(); });
  for(auto& option : state.options) {
    optionList.append(ComboButtonItem().setText(option));
  }
  optionList.doChange();  //updates response.option to point to the default (first) option
  fileName.setVisible(state.action == "saveFile").onActivate([&] { accept(); });
  acceptButton.onActivate([&] { accept(); });
  if(state.action == "openFile" || state.action == "openFiles" || state.action == "openFolder") acceptButton.setText("Open");
  if(state.action == "saveFile") acceptButton.setText("Save");
  if(state.action == "selectFolder") acceptButton.setText("Select");
  cancelButton.setText("Cancel").onActivate([&] { window.setModal(false); });

  if(!state.filters) state.filters.append("All|*");
  for(auto& filter : state.filters) {
    auto part = filter.split("|", 1L);
    filters.append(part.right().split(":"));
  }

  setPath(state.path);

  window.onClose([&] { window.setModal(false); });
  window.setTitle(state.title);
  window.setSize({640, 480});
  window.setCentered(state.parent);
  window.setDismissable();
  window.setVisible();
  view.setFocused();
  window.setModal();
  window.setVisible(false);

  return response;
}

auto BrowserDialogWindow::setPath(string path) -> void {
  path.transform("\\", "/");
  if((path || Path::root() == "/") && !path.endsWith("/")) path.append("/");
  pathName.setText(state.path = path);

  view.reset();

  auto contents = directory::icontents(path);
  bool folderMode = state.action == "openFolder";

  for(auto content : contents) {
    if(!content.endsWith("/")) continue;
    content.trimRight("/");
    if(folderMode && isMatch(content)) continue;

    view.append(ListViewItem().setText(content).setIcon(Icon::Emblem::Folder));
  }

  for(auto content : contents) {
    if(content.endsWith("/") != folderMode) continue;  //file mode shows files; folder mode shows folders
    content.trimRight("/");
    if(!isMatch(content)) continue;

    view.append(ListViewItem().setText(content).setIcon(folderMode ? Icon::Action::Open : Icon::Emblem::File));
  }

  Application::processEvents();
  view->resizeColumns();  //todo: on Windows, adding items may add vertical scrollbar; this hack corrects column width
  view.setFocused().doChange();
}

//

BrowserDialog::BrowserDialog() {
}

auto BrowserDialog::openFile() -> string {
  state.action = "openFile";
  if(!state.title) state.title = "Open File";
  if(auto result = _run()) return result.left();
  return {};
}

auto BrowserDialog::openFiles() -> string_vector {
  state.action = "openFiles";
  if(!state.title) state.title = "Open Files";
  if(auto result = _run()) return result;
  return {};
}

auto BrowserDialog::openFolder() -> string {
  state.action = "openFolder";
  if(!state.title) state.title = "Open Folder";
  if(auto result = _run()) return result.left();
  return {};
}

auto BrowserDialog::option() -> string {
  return response.option;
}

auto BrowserDialog::saveFile() -> string {
  state.action = "saveFile";
  if(!state.title) state.title = "Save File";
  if(auto result = _run()) return result.left();
  return {};
}

auto BrowserDialog::selected() -> string_vector {
  return response.selected;
}

auto BrowserDialog::selectFolder() -> string {
  state.action = "selectFolder";
  if(!state.title) state.title = "Select Folder";
  if(auto result = _run()) return result.left();
  return {};
}

auto BrowserDialog::setFilters(const string_vector& filters) -> type& {
  state.filters = filters;
  return *this;
}

auto BrowserDialog::setOptions(const string_vector& options) -> type& {
  state.options = options;
  return *this;
}

auto BrowserDialog::setParent(const sWindow& parent) -> type& {
  state.parent = parent;
  return *this;
}

auto BrowserDialog::setPath(const string& path) -> type& {
  state.path = path;
  return *this;
}

auto BrowserDialog::setTitle(const string& title) -> type& {
  state.title = title;
  return *this;
}

auto BrowserDialog::_run() -> string_vector {
  if(!state.path) state.path = Path::user();
  response = BrowserDialogWindow(state).run();
  return response.selected;
}

#endif
