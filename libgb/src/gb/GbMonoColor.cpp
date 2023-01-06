#include "gb/GbMonoColor.h"
#include "exception/TGenericException.h"
#include <cmath>
#include <iostream>

using namespace BlackT;

namespace Gb {


const double GbMonoColor::colorConversionFactor
  = ((double)255 / (double)(numColorLevels - 1));



GbMonoColor::GbMonoColor()
  : color_(color_white) {
  
}

bool GbMonoColor::operator==(const GbMonoColor& other) const {
  return (color_ == other.color_);
}

bool GbMonoColor::operator!=(const GbMonoColor& other) const {
  return !(color_ == other.color_);
}

GbMonoColor::ColorValue GbMonoColor::color() const {
  return color_;
}

void GbMonoColor::setColor(ColorValue color__) {
  color_ = color__;
}

BlackT::TColor GbMonoColor::toRealColor() const {
  // note that color level is "inverted" compared to normal
  // (0 = pure white, 3 = pure black),
  // so we have to flip the raw value before doing the conversion
  int level = (numColorLevels - 1 - static_cast<int>(color_))
                  * colorConversionFactor;
  return TColor(level, level, level, TColor::fullAlphaOpacity);
  
/*  switch (color_) {
  case color_white:
    return TColor(0x00, 0x00, 0x00, TColor::fullAlphaOpacity);
    break;
  case color_lightGray:
    return TColor(0x55, 0x55, 0x55, TColor::fullAlphaOpacity);
    break;
  case color_darkGray:
    return TColor(0xAA, 0xAA, 0xAA, TColor::fullAlphaOpacity);
    break;
  case color_black:
    return TColor(0xFF, 0xFF, 0xFF, TColor::fullAlphaOpacity);
    break;
  default:
    throw TGenericException(T_SRCANDLINE,
                            "GbMonoColor::toRealColor()",
                            "impossible switch case");
    break;
  } */
}

void GbMonoColor::fromRealColor(BlackT::TColor color__) {
  // use R component only
  int level = color__.r();
  // 4 possible gray levels = divide by 0x55 to get a double from 0 to 3
  double target = (double)level / colorConversionFactor;
  // round to closest matching integer level
  int realLevel = std::round(target);
  
  // sanity
  if ((realLevel < 0) || (realLevel >= numColorLevels)) {
    throw TGenericException(T_SRCANDLINE,
                            "GbMonoColor::fromRealColor()",
                            "impossible color conversion");
  }
  
  // invert target to get raw representation
  color_ = static_cast<ColorValue>(numColorLevels - 1 - realLevel);
}


}
