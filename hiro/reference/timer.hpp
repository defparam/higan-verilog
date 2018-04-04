namespace phoenix {

struct pTimer : public pObject {
  Timer& timer;

  void setEnabled(bool enabled);
  void setInterval(unsigned interval);

  pTimer(Timer& timer) : pObject(timer), timer(timer) {}
  void constructor();
  void destructor();
};

}
