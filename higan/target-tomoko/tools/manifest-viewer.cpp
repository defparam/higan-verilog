ManifestViewer::ManifestViewer(TabFrame* parent) : TabFrameItem(parent) {
  setIcon(Icon::Emblem::Text);
  setText("Manifest Viewer");

  layout.setMargin(5);
  manifestView.setEditable(false).setWordWrap(false).setFont(Font().setFamily(Font::Mono));
}

auto ManifestViewer::doRefresh() -> void {
  manifestView.setText("");
  if(emulator) manifestView.setText(emulator->manifest());
}
