#include "gb/GbVram.h"
#include "util/TStringConversion.h"
#include "exception/TGenericException.h"
#include <string>

using namespace BlackT;

namespace Gb {


GbVram::GbVram() { }

const GbPattern& GbVram::getPattern(int index) const {
  if (index >= numPatterns) {
    throw TGenericException(T_SRCANDLINE,
                            "GbVram::getPattern(int)",
                            (std::string("Out-of-range index: ")
                              + TStringConversion::intToString(index))
                              .c_str());
  }
  
  return patterns[index];
}

GbPattern GbVram::setPattern(int index, const GbPattern& pattern) {
  if (index >= numPatterns) {
    throw TGenericException(T_SRCANDLINE,
                            "GbVram::setPattern(int)",
                            (std::string("Out-of-range index: ")
                              + TStringConversion::intToString(index))
                              .c_str());
  }
  
  return patterns[index] = pattern;
}

const GbMonoPaletteLine& GbVram::getMonoPalette() const {
  return monoPalette;
}

/*const GbPalette& GbVram::getPalette(int index) const {
  if (index == 0) return getTilePalette();
  else return getSpritePalette();
}

GbPalette GbVram::setPalette(int index, const GbPalette& palette) {
  if (index == 0) return setTilePalette(palette);
  else return setSpritePalette(palette);
}

const GbPalette& GbVram::getTilePalette() const {
  return tilePalette;
}

GbPalette GbVram::setTilePalette(const GbPalette& tilePalette__) {
  tilePalette = tilePalette__;
  return tilePalette;
}

const GbPalette& GbVram::getSpritePalette() const {
  return spritePalette;
}

GbPalette GbVram::setSpritePalette(const GbPalette& spritePalette__) {
  spritePalette = spritePalette__;
  return spritePalette;
}*/
  
//void GbVram::read(const char* data, int numPatterns,
//                  int startPattern) {
//  for (int i = 0; i < numPatterns; i++) {
//    patterns[startPattern + i].read(data + (i * GbPattern::size));
//  }
//}

void GbVram::read(BlackT::TStream& ifs, int numPatterns,
                   int startPattern) {
  for (int i = 0; i < numPatterns; i++) {
    patterns[startPattern + i].read(ifs);
  }
}


}
