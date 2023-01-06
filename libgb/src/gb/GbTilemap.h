#ifndef GBTILEMAP_H
#define GBTILEMAP_H


#include "util/TArray.h"
#include "util/TTwoDArray.h"
#include "util/TByte.h"
#include "util/TGraphic.h"
#include "gb/GbTileId.h"

namespace Gb {


class GbTilemap {
public:
  
  GbTilemap();
  
  void resize(int w, int h);
  const GbTileId& getTileId(int x, int y) const;
  void setTileId(int x, int y, const GbTileId& tileId);
  
  void read(const char* src, int w, int h);
  
  void toColorGraphic(BlackT::TGraphic& dst,
                      const GbVram& vram,
                      bool gbcMode = false
//                      const GbPalette& pal
                      );
  
  void toGrayscaleGraphic(BlackT::TGraphic& dst,
                      const GbVram& vram,
                      bool gbcMode = false);
  
  BlackT::TTwoDArray<GbTileId> tileIds;
protected:
};


}


#endif
