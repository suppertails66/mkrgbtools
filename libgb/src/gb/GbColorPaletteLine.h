#ifndef GBCOLORPALETTELINE_H
#define GBCOLORPALETTELINE_H


#include "gb/GbPaletteLine.h"
#include "gb/GbColor.h"
#include "util/TArray.h"

namespace Gb {


class GbColorPaletteLine : public GbPaletteLine {
public:
  GbColorPaletteLine();
  
  virtual BlackT::TColor getRealColorAtIndex(int index) const;
  virtual void setRealColorAtIndex(int index, BlackT::TColor color);
  virtual int indexOfColor(BlackT::TColor color) const;
protected:
  BlackT::TArray<GbColor> colors_;
  
};


}


#endif
