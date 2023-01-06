#include "gb/GbColorPaletteLine.h"
#include <iostream>

using namespace BlackT;

namespace Gb {


GbColorPaletteLine::GbColorPaletteLine()
  : colors_(numColors()) {
  
}

BlackT::TColor GbColorPaletteLine::getRealColorAtIndex(int index) const {
  return colors_[index].toRealColor();
}

void GbColorPaletteLine::setRealColorAtIndex(int index, BlackT::TColor color) {
  colors_[index].fromRealColor(color);
}

int GbColorPaletteLine::indexOfColor(BlackT::TColor color) const {
  GbColor targetColor;
  targetColor.fromRealColor(color);

  for (int i = 0; i < numColors(); i++) {
    if (colors_[i] == targetColor) return i;
  }
  
  return -1;
}


}
