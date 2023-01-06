#ifndef GBPATTERN_H
#define GBPATTERN_H


#include "util/TStream.h"
#include "util/TTwoDArray.h"
#include "util/TGraphic.h"
#include "gb/GbPaletteLine.h"
#include <cstdlib>

namespace Gb {


class GbPattern {
public:
  const static int w = 8;
  const static int h = 8;
  const static int size = 16;
  const static int bitplanesPerRow = 2;

  GbPattern();
  
  bool operator==(const GbPattern& other) const;
  
  void read(BlackT::TStream& ifs);
  void write(BlackT::TStream& ofs) const;
  
  void toGraphic(BlackT::TGraphic& dst,
                 const GbPaletteLine* palette = NULL,
                 int xOffset = 0, int yOffset = 0,
                 bool transparency = false,
                 bool transparencyIsBlit = false) const;
  
  int fromGraphic(const BlackT::TGraphic& src,
                  const GbPaletteLine* line = NULL,
                  int xoffset = 0, int yoffset = 0);
  
/*  void approximateGraphic(const BlackT::TGraphic& src,
                 GbPalette& dstPalette,
                 bool* colorsUsed,
                 const bool* colorsAvailable,
                 int xOffset = 0, int yOffset = 0,
                 bool transparency = false,
                 bool colorsLocked = false,
                 bool ggMode = false); */
  
/*  void fromGrayscaleGraphic(const BlackT::TGraphic& src,
                        int xoffset, int yoffset);
  
  int fromColorGraphic(const BlackT::TGraphic& src,
                        const GbPalette& line,
                        int xoffset, int yoffset);
  
  int fromColorGraphicGG(const BlackT::TGraphic& src,
                        const GbPalette& line,
                        int xoffset, int yoffset); */
                 
  bool isEmpty() const;
  
  int data(int x, int y);
  int data(int x, int y) const;
  void setData(int x, int y, BlackT::TByte value);
  
  void clear();
  void flipV();
  void flipH();
protected:
  const static int numColorValues = 4;
//  const static int grayscaleConversionFactor = 0x55;
  const static int grayscaleConversionFactorIn = 0x44;
  const static int grayscaleConversionFactorOut = 0x55;
  BlackT::TTwoDArray<BlackT::TByte> data_;
  
};


}


#endif
