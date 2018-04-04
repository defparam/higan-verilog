#include "oam.cpp"

auto PPU::Object::addressReset() -> void {
  ppu.io.oamAddress = ppu.io.oamBaseAddress;
  setFirstSprite();
}

auto PPU::Object::setFirstSprite() -> void {
  io.firstSprite = !ppu.io.oamPriority ? 0 : ppu.io.oamAddress >> 2;
}

auto PPU::Object::frame() -> void {
  io.timeOver = false;
  io.rangeOver = false;
}

auto PPU::Object::scanline() -> void {
  t.x = 0;
  t.y = ppu.vcounter();

  t.itemCount = 0;
  t.tileCount = 0;

  t.active = !t.active;
  auto oamItem = t.item[t.active];
  auto oamTile = t.tile[t.active];

  if(t.y == ppu.vdisp() && !ppu.io.displayDisable) addressReset();
  if(t.y >= ppu.vdisp() - 1) return;

  for(auto n : range(32)) oamItem[n].valid = false;  //default to invalid
  for(auto n : range(34)) oamTile[n].valid = false;  //default to invalid

  for(auto n : range(128)) {
    uint7 sprite = io.firstSprite + n;
    if(!onScanline(oam.object[sprite])) continue;
    if(t.itemCount++ >= 32) break;
    oamItem[t.itemCount - 1] = {true, sprite};
  }

  if(t.itemCount > 0 && oamItem[t.itemCount - 1].valid) {
    ppu.latch.oamAddress = 0x0200 + (oamItem[t.itemCount - 1].index >> 2);
  }
}

auto PPU::Object::onScanline(PPU::OAM::Object& sprite) -> bool {
  if(sprite.x > 256 && (sprite.x + sprite.width() - 1) < 512) return false;
  int height = sprite.height() >> io.interlace;
  if(t.y >= sprite.y && t.y < (sprite.y + height)) return true;
  if((sprite.y + height) >= 256 && t.y < ((sprite.y + height) & 255)) return true;
  return false;
}

auto PPU::Object::run() -> void {
  output.above.priority = 0;
  output.below.priority = 0;

  auto oamTile = t.tile[!t.active];
  uint x = t.x++;

  for(auto n : range(34)) {
    const auto& tile = oamTile[n];
    if(!tile.valid) break;

    int px = x - (int9)tile.x;
    if(px & ~7) continue;

    uint color = 0, shift = tile.hflip ? px : 7 - px;
    color += tile.data >> (shift +  0) & 1;
    color += tile.data >> (shift +  7) & 2;
    color += tile.data >> (shift + 14) & 4;
    color += tile.data >> (shift + 21) & 8;

    if(color) {
      if(io.aboveEnable) {
        output.above.palette = tile.palette + color;
        output.above.priority = io.priority[tile.priority];
      }

      if(io.belowEnable) {
        output.below.palette = tile.palette + color;
        output.below.priority = io.priority[tile.priority];
      }
    }
  }
}

auto PPU::Object::tilefetch() -> void {
  auto oamItem = t.item[t.active];
  auto oamTile = t.tile[t.active];

  for(int i = 31; i >= 0; i--) {
    if(!oamItem[i].valid) continue;
    const auto& sprite = oam.object[oamItem[i].index];

    uint tileWidth = sprite.width() >> 3;
    int x = sprite.x;
    int y = (t.y - sprite.y) & 0xff;
    if(io.interlace) y <<= 1;

    if(sprite.vflip) {
      if(sprite.width() == sprite.height()) {
        y = (sprite.height() - 1) - y;
      } else if(y < sprite.width()) {
        y = (sprite.width() - 1) - y;
      } else {
        y = sprite.width() + ((sprite.width() - 1) - (y - sprite.width()));
      }
    }

    if(io.interlace) {
      y = !sprite.vflip ? y + ppu.field() : y - ppu.field();
    }

    x &= 511;
    y &= 255;

    uint16 tiledataAddress = io.tiledataAddress;
    uint16 chrx = (sprite.character >> 0) & 15;
    uint16 chry = (sprite.character >> 4) & 15;
    if(sprite.nameselect) {
      tiledataAddress += (256 * 16) + (io.nameselect << 12);
    }
    chry  += (y >> 3);
    chry  &= 15;
    chry <<= 4;

    for(uint tx : range(tileWidth)) {
      uint sx = (x + (tx << 3)) & 511;
      if(x != 256 && sx >= 256 && (sx + 7) < 512) continue;
      if(t.tileCount++ >= 34) break;

      uint n = t.tileCount - 1;
      oamTile[n].valid = true;
      oamTile[n].x = sx;
      oamTile[n].priority = sprite.priority;
      oamTile[n].palette = 128 + (sprite.palette << 4);
      oamTile[n].hflip = sprite.hflip;

      uint mx = !sprite.hflip ? tx : (tileWidth - 1) - tx;
      uint pos = tiledataAddress + ((chry + ((chrx + mx) & 15)) << 4);
      uint16 addr = (pos & 0xfff0) + (y & 7);

      oamTile[n].data.bits( 0,15) = ppu.vram[addr + 0];
      ppu.step(2);

      oamTile[n].data.bits(16,31) = ppu.vram[addr + 8];
      ppu.step(2);
    }
  }

  if(t.tileCount < 34) ppu.step((34 - t.tileCount) * 4);
  io.timeOver  |= (t.tileCount > 34);
  io.rangeOver |= (t.itemCount > 32);
}

auto PPU::Object::power() -> void {
  for(auto& object : oam.object) {
    object.x = 0;
    object.y = 0;
    object.character = 0;
    object.nameselect = 0;
    object.vflip = 0;
    object.hflip = 0;
    object.priority = 0;
    object.palette = 0;
    object.size = 0;
  }

  t.x = 0;
  t.y = 0;

  t.itemCount = 0;
  t.tileCount = 0;

  t.active = 0;
  for(auto p : range(2)) {
    for(auto n : range(32)) {
      t.item[p][n].valid = false;
      t.item[p][n].index = 0;
    }
    for(auto n : range(34)) {
      t.tile[p][n].valid = false;
      t.tile[p][n].x = 0;
      t.tile[p][n].priority = 0;
      t.tile[p][n].palette = 0;
      t.tile[p][n].hflip = 0;
      t.tile[p][n].data = 0;
    }
  }

  io.aboveEnable = random();
  io.belowEnable = random();
  io.interlace = random();

  io.baseSize = random();
  io.nameselect = random();
  io.tiledataAddress = (random() & 7) << 13;
  io.firstSprite = 0;

  for(auto& p : io.priority) p = 0;

  io.timeOver = false;
  io.rangeOver = false;

  output.above.palette = 0;
  output.above.priority = 0;
  output.below.palette = 0;
  output.below.priority = 0;
}
