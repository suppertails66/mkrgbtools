#include "gb/GbPattern.h"
#include "gb/GbMonoPaletteLine.h"
#include "gb/GbColorPaletteLine.h"
#include "util/TIfstream.h"
#include "util/TBufStream.h"
#include "util/TGraphic.h"
#include "util/TPngConversion.h"
#include "util/TOpt.h"
#include <iostream>

using namespace std;
using namespace BlackT;
using namespace Gb;

int patternsPerRow = 16;

int main(int argc, char* argv[]) {
  if (argc < 3) {
    cout << "Game Boy raw graphics dumper" << endl;
    cout << "Usage: " << argv[0]
      << " <infile> <outfile> [options]" << endl;
    cout << "Options: " << endl;
    cout << "  p     Set palette (default: grayscale)" << endl;
    cout << "  o     Set starting offset (default: 0)" << endl;
    cout << "  n     Set number of patterns (default: all)" << endl;
    return 0;
  }
  
  TIfstream ifs(argv[1], ios_base::binary);
  
  int offset = 0;
  TOpt::readNumericOpt(argc, argv, "-o", &offset);
  
  ifs.seek(offset);
  
  int numPatterns = ifs.remaining() / GbPattern::size;
  TOpt::readNumericOpt(argc, argv, "-n", &numPatterns);
                             
  GbPaletteLine* palP = NULL;
  GbColorPaletteLine pal;
  if (TOpt::getOpt(argc, argv, "-p") != NULL) {
    TIfstream ifs(TOpt::getOpt(argc, argv, "-p"), ios_base::binary);
//    pal.readGG(ifs);
//    palP = &pal;
  }
  
  int outW = patternsPerRow * GbPattern::w;
  int outH = numPatterns / patternsPerRow;
  if ((numPatterns % patternsPerRow)) ++outH;
  outH *= GbPattern::h;
  
  TGraphic g(outW, outH);
  g.clearTransparent();
  
  for (int i = 0; i < numPatterns; i++) {
    int x = (i % patternsPerRow) * GbPattern::w;
    int y = (i / patternsPerRow) * GbPattern::h;
  
    GbPattern pattern;
    pattern.read(ifs);
    pattern.toGraphic(g, palP, x, y,
                      false, false);
  }
  
  TPngConversion::graphicToRGBAPng(string(argv[2]), g);
  
  return 0;
}
