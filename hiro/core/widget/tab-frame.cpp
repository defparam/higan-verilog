#if defined(Hiro_TabFrame)

auto mTabFrame::allocate() -> pObject* {
  return new pTabFrame(*this);
}

auto mTabFrame::destruct() -> void {
  for(auto& item : state.items) item->destruct();
  mWidget::destruct();
}

//

auto mTabFrame::append(sTabFrameItem item) -> type& {
  if(!state.items) item->state.selected = true;
  state.items.append(item);
  item->setParent(this, itemCount() - 1);
  signal(append, item);
  return *this;
}

auto mTabFrame::doChange() const -> void {
  if(state.onChange) return state.onChange();
}

auto mTabFrame::doClose(sTabFrameItem item) const -> void {
  if(state.onClose) return state.onClose(item);
}

auto mTabFrame::doMove(sTabFrameItem from, sTabFrameItem to) const -> void {
  if(state.onMove) return state.onMove(from, to);
}

auto mTabFrame::item(unsigned position) const -> TabFrameItem {
  if(position < itemCount()) return state.items[position];
  return {};
}

auto mTabFrame::itemCount() const -> unsigned {
  return state.items.size();
}

auto mTabFrame::items() const -> vector<TabFrameItem> {
  vector<TabFrameItem> items;
  for(auto& item : state.items) items.append(item);
  return items;
}

auto mTabFrame::navigation() const -> Navigation {
  return state.navigation;
}

auto mTabFrame::onChange(const function<void ()>& callback) -> type& {
  state.onChange = callback;
  return *this;
}

auto mTabFrame::onClose(const function<void (TabFrameItem)>& callback) -> type& {
  state.onClose = callback;
  return *this;
}

auto mTabFrame::onMove(const function<void (TabFrameItem, TabFrameItem)>& callback) -> type& {
  state.onMove = callback;
  return *this;
}

auto mTabFrame::remove(sTabFrameItem item) -> type& {
  auto offset = item->offset();
  item->setParent();
  signal(remove, item);
  state.items.remove(item->offset());
  for(auto n : range(offset, itemCount())) {
    state.items[n]->adjustOffset(-1);
  }
  return *this;
}

auto mTabFrame::reset() -> type& {
  while(state.items) remove(state.items.right());
  return *this;
}

auto mTabFrame::selected() const -> TabFrameItem {
  for(auto& item : state.items) {
    if(item->selected()) return item;
  }
  return {};
}

auto mTabFrame::setNavigation(Navigation navigation) -> type& {
  state.navigation = navigation;
  signal(setNavigation, navigation);
  return *this;
}

auto mTabFrame::setParent(mObject* parent, signed offset) -> type& {
  for(auto n : rrange(state.items)) state.items[n]->destruct();
  mObject::setParent(parent, offset);
  for(auto& item : state.items) item->setParent(this, item->offset());
  return *this;
}

#endif
