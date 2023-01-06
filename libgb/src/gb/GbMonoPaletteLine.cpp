#include "gb/GbMonoPaletteLine.h"
#include <iostream>

using namespace BlackT;

namespace Gb {


GbMonoPaletteLine::GbMonoPaletteLine()
  : colors_(numColors()) {
  
}

BlackT::TColor GbMonoPaletteLine::getRealColorAtIndex(int index) const {
  return colors_[index].toRealColor();
}

void GbMonoPaletteLine::setRealColorAtIndex(int index, BlackT::TColor color) {
  colors_[index].fromRealColor(color);
}

int GbMonoPaletteLine::indexOfColor(BlackT::TColor color) const {
  GbMonoColor targetColor;
  targetColor.fromRealColor(color);

  for (int i = 0; i < numColors(); i++) {
    if (colors_[i] == targetColor) return i;
  }
  
  return -1;
}


}
