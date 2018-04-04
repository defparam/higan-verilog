struct OAM {
  auto read(uint10 addr) -> uint8;
  auto write(uint10 addr, uint8 data) -> void;

  struct Object {
    alwaysinline auto width() const -> uint;
    alwaysinline auto height() const -> uint;

    uint9 x;
    uint8 y;
    uint8 character;
    uint1 nameselect;
    uint1 vflip;
    uint1 hflip;
    uint2 priority;
    uint3 palette;
    uint1 size;
  } object[128];
};

struct Object {
  alwaysinline auto addressReset() -> void;
  alwaysinline auto setFirstSprite() -> void;
  auto frame() -> void;
  auto scanline() -> void;
  auto run() -> void;
  auto tilefetch() -> void;
  auto power() -> void;

  auto onScanline(PPU::OAM::Object&) -> bool;

  auto serialize(serializer&) -> void;

  OAM oam;

  struct IO {
    bool aboveEnable;
    bool belowEnable;
    bool interlace;

    uint3  baseSize;
    uint2  nameselect;
    uint16 tiledataAddress;
    uint7  firstSprite;

    uint priority[4];

    bool timeOver;
    bool rangeOver;
  } io;

  struct Item {
    bool  valid;
    uint7 index;
  };

  struct Tile {
    bool   valid;
    uint9  x;
    uint2  priority;
    uint8  palette;
    uint1  hflip;
    uint32 data;
  };

  struct State {
    uint x;
    uint y;

    uint itemCount;
    uint tileCount;

    bool active;
    Item item[2][32];
    Tile tile[2][34];
  } t;

  struct Output {
    struct Pixel {
      uint  priority;  //0 = none (transparent)
      uint8 palette;
    } above, below;
  } output;

  friend class PPU;
};
