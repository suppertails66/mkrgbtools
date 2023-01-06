#ifndef GBTILEID_H
#define GBTILEID_H


#include "util/TByte.h"
#include "util/TGraphic.h"
#include "gb/GbVram.h"
//#include "gb/GbPalette.h"

namespace Gb {


class GbTileId {
public:
  const static int size = 1;
  
  GbTileId();
  
  void read(const char* data);
  void write(char* data) const;
  
  void toColorGraphic(BlackT::TGraphic& dst,
                      const GbVram& vram,
//                      const GbPalette& pal,
                      int x = 0, int y = 0,
                      bool gbcMode = false) const;
  
  void toGrayscaleGraphic(BlackT::TGraphic& dst,
                      const GbVram& vram,
                      int x = 0, int y = 0,
                      bool gbcMode = false) const;
  
  int pattern;
  bool priority;
  int palette;
  bool vflip;
  bool hflip;
  bool isBank1;
  
protected:
  
};


}


#endif
