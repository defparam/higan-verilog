#if defined(Hiro_VerticalLayout)

struct mVerticalLayout : mLayout {
  using type = mVerticalLayout;
  using mLayout::append;
  using mLayout::remove;

  auto append(sSizable sizable, Size size, float spacing = 5) -> type&;
  auto modify(sSizable sizable, Size size, float spacing = 5) -> type&;
  auto minimumSize() const -> Size override;
  auto remove(sSizable sizable) -> type& override;
  auto reset() -> type& override;
  auto setAlignment(float alignment = 0.0) -> type&;
  auto setEnabled(bool enabled = true) -> type& override;
  auto setFont(const Font& font = {}) -> type& override;
  auto setGeometry(Geometry geometry) -> type& override;
  auto setMargin(float margin = 0) -> type&;
  auto setSpacing(float spacing = 5) -> type&;
  auto setVisible(bool visible = true) -> type& override;

  struct Settings {
    float alignment = 0.0;
    float margin = 0;
    float spacing = 5;
  } settings;

  struct Property : Size {
    Property() = default;
    Property(float width, float height, float spacing) : Size(width, height), _spacing(spacing) {}
    auto setSpacing(float spacing) -> Property& { return _spacing = spacing, *this; }
    auto spacing() const -> float { return _spacing; }
    float _spacing = 0;
  };
  vector<Property> properties;
};

#endif
