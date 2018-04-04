#if defined(Hiro_HexEdit)

@interface CocoaHexEdit : NSScrollView {
@public
  phoenix::HexEdit* hexEdit;
}
-(id) initWith:(phoenix::HexEdit&)hexEdit;
@end

namespace hiro {

struct pHexEdit : public pWidget {
  HexEdit& hexEdit;
  CocoaHexEdit* cocoaHexEdit = nullptr;

  void setBackgroundColor(Color color);
  void setColumns(unsigned columns);
  void setForegroundColor(Color color);
  void setLength(unsigned length);
  void setOffset(unsigned offset);
  void setRows(unsigned rows);
  void update();

  pHexEdit(HexEdit& hexEdit) : pWidget(hexEdit), hexEdit(hexEdit) {}
  void constructor();
  void destructor();
};

}

#endif
