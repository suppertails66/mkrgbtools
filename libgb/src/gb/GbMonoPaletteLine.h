#ifndef GBMONOPALETTELINE_H
#define GBMONOPALETTELINE_H


#include "gb/GbPaletteLine.h"
#include "gb/GbMonoColor.h"
#include "util/TArray.h"

namespace Gb {


class GbMonoPaletteLine : public GbPaletteLine {
public:
  GbMonoPaletteLine();
  
  virtual BlackT::TColor getRealColorAtIndex(int index) const;
  virtual void setRealColorAtIndex(int index, BlackT::TColor color);
  virtual int indexOfColor(BlackT::TColor color) const;
protected:
  BlackT::TArray<GbMonoColor> colors_;
};


}


#endif
