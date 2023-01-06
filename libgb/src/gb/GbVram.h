#ifndef GBVRAM_H
#define GBVRAM_H


#include "util/TByte.h"
#include "util/TStream.h"
#include "gb/GbPattern.h"
#include "gb/GbMonoPaletteLine.h"

namespace Gb {


class GbVram {
public:
  const static int numPatterns = 0x200;
  const static int numBytes = numPatterns * GbPattern::size;
  const static int numPaletteLines = 1;

  GbVram();
  
  const GbPattern& getPattern(int index) const;
  GbPattern setPattern(int index, const GbPattern& pattern);
  
  const GbMonoPaletteLine& getMonoPalette() const;
/*  const GbPalette& getPalette(int index) const;
  GbPalette setPalette(int index, const GbPalette& palette);
  const GbPalette& getTilePalette() const;
  GbPalette setTilePalette(const GbPalette& tilePalette__);
  const GbPalette& getSpritePalette() const;
  GbPalette setSpritePalette(const GbPalette& spritePalette__);*/
  
//  void read(const char* data, int numPatterns,
//            int startPattern = 0);
  
  void read(BlackT::TStream& ifs, int numPatterns,
            int startPattern = 0);
protected:
  
  GbPattern patterns[numPatterns];
  GbMonoPaletteLine monoPalette;
//  GbPalette tilePalette;
//  GbPalette spritePalette;
  
};


}


#endif
