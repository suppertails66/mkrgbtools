#ifndef GBCOLOR_H
#define GBCOLOR_H


#include "util/TStream.h"
#include "util/TColor.h"

namespace Gb {


class GbColor {
public:
  GbColor();
  
  bool operator==(const GbColor& other) const;
  bool operator!=(const GbColor& other) const;
  
  int rawColor() const;
  void setRawColor(int rawColor__);
  
  BlackT::TColor toRealColor() const;
  void fromRealColor(BlackT::TColor color__);
protected:
  const static int numColorLevels = 32;
  // divide 8-bit color level by this to get corresponding ColorValue
  const static double colorConversionFactor;
  
  const static int rMask  = (0x1F) << 0;
  const static int rShift = 5 * 0;
  const static int gMask  = (0x1F) << 5;
  const static int gShift = 5 * 1;
  const static int bMask  = (0x1F) << 10;
  const static int bShift = 5 * 2;
  
  int rawColor_;
};


}


#endif
