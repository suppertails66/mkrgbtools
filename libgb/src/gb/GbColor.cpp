#include "gb/GbColor.h"
#include <cmath>
#include <iostream>

using namespace BlackT;

namespace Gb {


const double GbColor::colorConversionFactor
  = ((double)255 / (double)(numColorLevels - 1));


GbColor::GbColor()
  : rawColor_(0) {
  
}

bool GbColor::operator==(const GbColor& other) const {
  return (rawColor_ == other.rawColor_);
}

bool GbColor::operator!=(const GbColor& other) const {
  return !(rawColor_ == other.rawColor_);
}

int GbColor::rawColor() const {
  return rawColor_;
}

void GbColor::setRawColor(int rawColor__) {
  rawColor_ = rawColor__;
}

BlackT::TColor GbColor::toRealColor() const {
  int rawR = (rawColor_ & rMask) >> rShift;
  int rawG = (rawColor_ & gMask) >> gShift;
  int rawB = (rawColor_ & bMask) >> bShift;
  
  double targetR = std::round((double)rawR * colorConversionFactor);
  double targetG = std::round((double)rawG * colorConversionFactor);
  double targetB = std::round((double)rawB * colorConversionFactor);
  
  return TColor(targetR, targetG, targetB, TColor::fullAlphaOpacity);
}

void GbColor::fromRealColor(BlackT::TColor color__) {
  int rawR = std::round(((double)color__.r() / colorConversionFactor));
  int rawG = std::round(((double)color__.g() / colorConversionFactor));
  int rawB = std::round(((double)color__.b() / colorConversionFactor));
  
  rawColor_ = 0;
  rawColor_ |= (rawR << rShift);
  rawColor_ |= (rawG << gShift);
  rawColor_ |= (rawB << bShift);
}


}
