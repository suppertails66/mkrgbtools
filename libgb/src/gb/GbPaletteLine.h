#ifndef GBPALETTELINE_H
#define GBPALETTELINE_H


#include "util/TColor.h"

namespace Gb {


class GbPaletteLine {
public:
  /* gb/gbc color differences are stupid.
     since the underlying color data format differs between mono/color,
     rather than inheriting any data members or providing common functionality,
     this basically acts as an interface to allow the respective mono/color
     PaletteLine classes to be accessed in the same way so we can avoid having
     to e.g. write separate toColorGraphic and toMonoGraphic functions
     in GbPattern. */
  
  /**
   * Returns a truecolor conversion of the Nth color of the palette.
   *
   * @param index Index number of the target color.
   * @return Truecolor conversion of the target color.
   */
  virtual BlackT::TColor getRealColorAtIndex(int index) const =0;
  
  /**
   * Converts the Nth color of the palette to the closest possible
   * approximation of a given truecolor.
   * For grayscale mode, do not expect non-grayscale input colors to convert
   * correctly.
   *
   * @param index Target color index.
   * @param color Truecolor to approximate within the inheritor's limitations.
   */
  virtual void setRealColorAtIndex(int index, BlackT::TColor color) =0;
  
  virtual int indexOfColor(BlackT::TColor color) const =0;
  
  int numColors() const;
protected:
  GbPaletteLine();
};


}


#endif
