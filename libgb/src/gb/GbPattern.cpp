#include "gb/GbPattern.h"
#include "gbexp/GbNotEnoughSpaceException.h"
#include <cstdlib>
#include <iostream>

using namespace BlackT;

namespace Gb {


GbPattern::GbPattern()
  : data_(w, h) {
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
//      data_.data(i, j) = GbPalette::spriteTransparencyIndex;
      data_.data(i, j) = 0;
    }
  }
}

bool GbPattern::operator==(const GbPattern& other) const {
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      if (data_.data(i, j) != other.data_.data(i, j)) return false;
    }
  }
  
  return true;
}
  
void GbPattern::read(BlackT::TStream& ifs) {
  clear();
  
/*  Each Tile occupies 16 bytes, where each 2 bytes represent a line:

  Byte 0-1  First Line (Upper 8 pixels)
  Byte 2-3  Next Line
  etc.

  For each line, the first byte defines the least significant bits of the color
  numbers for each pixel, and the second byte defines the upper bits of the
  color numbers. In either case, Bit 7 is the leftmost pixel, and Bit 0 the
  rightmost.

  So, each pixel is having a color number in range from 0-3. */
  
  for (int j = 0; j < h; j++) {
    int dstmask = 0x01;
    for (int k = 0; k < bitplanesPerRow; k++) {
      TByte next = ifs.get();
      
      int mask = 0x80;
      for (int i = 0; i < w; i++) {
        if (next & mask) data_.data(i, j) |= dstmask;
        mask >>= 1;
      }
      
      dstmask <<= 1;
    }
  }
}

void GbPattern::write(BlackT::TStream& ofs) const {
  for (int j = 0; j < h; j++) {
    int srcmask = 0x01;
    for (int k = 0; k < bitplanesPerRow; k++) {
      TByte next = 0;
      
      int mask = 0x80;
      for (int i = 0; i < w; i++) {
        if ((data_.data(i, j) & srcmask) != 0) next |= mask;
        mask >>= 1;
      }
      
      ofs.put(next);
      srcmask <<= 1;
    }
  }
}

void GbPattern::toGraphic(BlackT::TGraphic& dst,
               const GbPaletteLine* palette,
               int xOffset, int yOffset,
               bool transparency,
               bool transparencyIsBlit) const {
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      int x = xOffset + i;
      int y = yOffset + j;
      
      if (x < 0) continue;
      if (y < 0) continue;
      if (x >= dst.w()) continue;
      if (y >= dst.h()) continue;
      
      int rawValue = data_.data(i, j);
      
      TColor outputColor;
      if (palette != NULL) {
        // color
        outputColor = palette->getRealColorAtIndex(rawValue);
      }
      else {
        // grayscale (inverted to get standard color representation)
        int level = (numColorValues - 1 - rawValue)
                      * grayscaleConversionFactorOut;
        outputColor = TColor(level, level, level, TColor::fullAlphaOpacity);
      }
      
      if (transparency && (rawValue == 0))
        outputColor.setA(TColor::fullAlphaTransparency);
      
      if (transparencyIsBlit && (rawValue == 0)) {
      
      }
      else {
        dst.setPixel(x, y, outputColor);
      }
    }
  }
}
  
int GbPattern::fromGraphic(const BlackT::TGraphic& src,
               const GbPaletteLine* line,
               int xoffset, int yoffset) {
  // note that this simply chooses _a_ color that matches the
  // one from the source (if one exists in the palette).
  // if the same color is present multiple times, you may get
  // a different index from the original, which could cause problems
  // with dynamic palettes
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      int x = xoffset + i;
      int y = yoffset + j;
    
      TColor color = src.getPixel(x, y);
      
      int raw = 0;
      if (line == NULL) {
        raw = color.r() / grayscaleConversionFactorIn;
        // invert to get standard color representation
        raw = (numColorValues - 1 - raw);
      }
      else {
        raw = line->indexOfColor(color);
      }
      
      if (raw < 0) return raw;
      data_.data(i, j) = raw;
    }
  }
  
  return 0;
}

/*void GbPattern::fromGrayscaleGraphic(const BlackT::TGraphic& src,
                      int xoffset, int yoffset) {
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      int x = xoffset + i;
      int y = yoffset + j;
      
      if (x < 0) continue;
      if (y < 0) continue;
      if (x >= src.w()) continue;
      if (y >= src.h()) continue;
      
      BlackT::TColor rawColor = src.getPixel(x, y);
      int level = rawColor.r() >> 4;
      data_.data(i, j) = level;
    }
  }
}

void GbPattern::approximateGraphic(const BlackT::TGraphic& src,
               GbPalette& dstPalette,
               bool* colorsUsed,
               const bool* colorsAvailable,
               int xOffset, int yOffset,
               bool transparency,
               bool colorsLocked,
               bool ggMode) {
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      int x = xOffset + i;
      int y = yOffset + j;
      
      // ignore pixels outside image dimensions
      if ((x < 0) || (x >= src.w()) || (y < 0) || (y >= src.h())) continue;
      
      TColor rawColor = src.getPixel(x, y);
      // handle transparency
      if (transparency && (rawColor.a() == TColor::fullAlphaTransparency)) {
        data_.data(i, j) = GbPalette::spriteTransparencyIndex;
        continue;
      }
      
      GbColor color;
      if (ggMode)
        color.approximateTrueColorGG(rawColor);
      else
        color.approximateTrueColor(rawColor);
      
      int index = -1;
      
      // check if any existing color matches target
      for (int i = 0; i < GbPalette::numColors; i++) {
        if (!colorsAvailable[i]) continue;
        if ((colorsUsed[i]) && (dstPalette.getColor(i) == color)) {
          index = i;
          break;
        }
      }
      
      // match found
      if (index != -1) {
        data_.data(i, j) = index;
        continue;
      }
      
      // match not found
      if (colorsLocked) {
        throw GbNotEnoughSpaceException(T_SRCANDLINE,
                                         "GbPattern::approximateGraphic()",
                                         "Graphic uses colors not in palette");
      }
      
      // add new color
      
//      std::cout << color.r() << " " << color.g() << " " << color.b() << std::endl;
      for (int i = 0; i < GbPalette::numColors; i++) {
        if (!colorsAvailable[i]) continue;
        if (!colorsUsed[i]) {
          colorsUsed[i] = true;
          index = i;
          dstPalette.setColor(index, color);
          break;
        }
      }
      
      if (index == -1) {
        throw GbNotEnoughSpaceException(T_SRCANDLINE,
                                         "GbPattern::approximateGraphic()",
                                         "Source data uses too many colors");
      }
      
      data_.data(i, j) = index;
    }
  }
}
  
int GbPattern::fromColorGraphic(const BlackT::TGraphic& src,
               const GbPalette& line,
               int xoffset, int yoffset) {
  
  // note that this simply chooses _a_ color that matches the
  // one from the source (if one exists in the palette).
  // if the same color is present multiple times, you may get
  // a different index from the original, which could cause problems
  // with dynamic palettes
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      int x = xoffset + i;
      int y = yoffset + j;
    
      TColor color = src.getPixel(x, y);
      int raw = line.indexOfColor(color);
      if (raw < 0) return raw;
      data_.data(i, j) = raw;
    }
  }
  
  return 0;
}
  
int GbPattern::fromColorGraphicGG(const BlackT::TGraphic& src,
               const GbPalette& line,
               int xoffset, int yoffset) {
  
  // note that this simply chooses _a_ color that matches the
  // one from the source (if one exists in the palette).
  // if the same color is present multiple times, you may get
  // a different index from the original, which could cause problems
  // with dynamic palettes
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      int x = xoffset + i;
      int y = yoffset + j;
    
      TColor color = src.getPixel(x, y);
      int raw = line.indexOfColorGG(color);
      if (raw < 0) return raw;
      data_.data(i, j) = raw;
    }
  }
  
  return 0;
} */

bool GbPattern::isEmpty() const {
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      if (data_.data(i, j) != 0) return false;
    }
  }
  
  return true;
}
  
int GbPattern::data(int x, int y) {
  return data_.data(x, y);
}
  
int GbPattern::data(int x, int y) const {
  return data_.data(x, y);
}

void GbPattern::setData(int x, int y, BlackT::TByte value) {
  data_.data(x, y) = value;
}
  
void GbPattern::clear() {
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      data_.data(i, j) = 0;
    }
  }
}

void GbPattern::flipV() {
  data_.flipV();
}

void GbPattern::flipH() {
  data_.flipH();
}


}
