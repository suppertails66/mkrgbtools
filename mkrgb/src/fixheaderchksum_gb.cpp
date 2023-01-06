#include "util/TBufStream.h"
#include <iostream>

using namespace std;
using namespace BlackT;

int main(int argc, char* argv[]) {
  if (argc < 3) {
    cout << "Game Boy header checksum fixer" << endl;
    cout << "Usage: " << argv[0]
      << " <infile> <outfile>" << endl;
    return 0;
  }
  
  std::string inFile(argv[1]);
  std::string outFile(argv[2]);
  
  TBufStream ifs;
  ifs.open(inFile.c_str());
  
  ifs.seek(0x134);
  int value = 0;
  for (int i = 0; i < 0x19; i++) {
    int next = ifs.readu8();
    value = value - next - 1;
  }
  
  ifs.seek(0x14D);
  ifs.writeu8(value & 0xFF);
  
  ifs.save(outFile.c_str());
  
  return 0;
}
