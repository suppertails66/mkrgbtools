#include "gb/GbTilemap.h"

using namespace BlackT;

namespace Gb {


GbTilemap::GbTilemap() { }
  
void GbTilemap::resize(int w, int h) {
  tileIds.resize(w, h);
}

const GbTileId& GbTilemap::getTileId(int x, int y) const {
  return tileIds.data(x, y);
}

void GbTilemap::setTileId(int x, int y, const GbTileId& tileId) {
  tileIds.data(x, y) = tileId;
}

void GbTilemap::read(const char* src, int w, int h) {
  resize(w, h);
  
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      tileIds.data(i, j).read(src);
      src += GbTileId::size;
    }
  }
}
  
void GbTilemap::toColorGraphic(BlackT::TGraphic& dst,
                    const GbVram& vram,
                    bool gbcMode
//                    const GbPalette& pal
                    ) {
  // TODO
/*  dst.resize(tileIds.w() * GbPattern::w,
             tileIds.h() * GbPattern::h);
  dst.clearTransparent();
  
  for (int j = 0; j < tileIds.h(); j++) {
    for (int i = 0; i < tileIds.w(); i++) {
      tileIds.data(i, j).toColorGraphic(
        dst, vram, (i * GbPattern::w), (j * GbPattern::h), gbcMode);
    }
  }*/
}
  
void GbTilemap::toGrayscaleGraphic(BlackT::TGraphic& dst,
                    const GbVram& vram,
                    bool gbcMode) {
  dst.resize(tileIds.w() * GbPattern::w,
             tileIds.h() * GbPattern::h);
  dst.clearTransparent();
  
  for (int j = 0; j < tileIds.h(); j++) {
    for (int i = 0; i < tileIds.w(); i++) {
      tileIds.data(i, j).toGrayscaleGraphic(
        dst, vram, (i * GbPattern::w), (j * GbPattern::h), gbcMode);
    }
  }
}


}
