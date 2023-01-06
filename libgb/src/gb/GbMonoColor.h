#ifndef GBMONOCOLOR_H
#define GBMONOCOLOR_H


#include "util/TStream.h"
#include "util/TColor.h"

namespace Gb {


class GbMonoColor {
public:
  // enumeration of possible (grayscale) color values
  enum ColorValue {
    color_white     = 0,
    color_lightGray = 1,
    color_darkGray  = 2,
    color_black     = 3
  };

  GbMonoColor();
  
  bool operator==(const GbMonoColor& other) const;
  bool operator!=(const GbMonoColor& other) const;
  
  ColorValue color() const;
  void setColor(ColorValue color__);
  
  BlackT::TColor toRealColor() const;
  void fromRealColor(BlackT::TColor color__);
protected:
  const static int numColorLevels = 4;
  // divide 8-bit color level by this to get corresponding ColorValue
  const static double colorConversionFactor;

  ColorValue color_;
};


}


#endif
