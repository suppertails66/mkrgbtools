#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TBufStream.h"
#include "util/TIniFile.h"
#include "util/TStringConversion.h"
#include "util/TFreeSpace.h"
#include "util/TFileManip.h"
#include "util/TArray.h"
#include "util/TByte.h"
#include "util/TGraphic.h"
#include "util/TPngConversion.h"
#include "util/TTwoDArray.h"
#include "gb/GbTilemap.h"
#include "gb/GbPattern.h"
//#include "gb/GbPalette.h"
#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <cstring>

using namespace std;
using namespace BlackT;
using namespace Gb;

// stupid and inefficient, but good enough for what we're doing
typedef vector<int> Blacklist;

// hack
class GbTilemapExt : public GbTilemap {
public:
  void resize(int w, int h) {
    isRaw.resize(w, h);
    GbTilemap::resize(w, h);
  }
  
  TTwoDArray<bool> isRaw;
};

// HACK: i don't know what the fuck happened but backgrounded tiles were
// completely broken in a way that the current implementation makes
// very difficult to fix "properly".
// as a workaround,
// this will be set to true if a call to processTile() returned a
// backgrounded tile and false otherwise.
bool lastProcessTileFoundBackgroundedTile = false;

int readIntString(const string& src, int* pos) {
  string numstr;
  while (*pos < src.size()) {
    // accept "x" for hex
    if (!isalnum(src[*pos]) && !(src[*pos] == 'x')) break;
    else {
      numstr += src[(*pos)++];
    }
  }
  
  if (*pos < src.size()) ++(*pos);
  
  return TStringConversion::stringToInt(numstr);
}

void readBlacklist(Blacklist& blacklist, const string& src) {
  int pos = 0;
  
//  std::cerr << "start" << std::endl;
  while ((pos < src.size())) {
    int next = readIntString(src, &pos);
    
    // check if this is a range
    if ((pos < src.size()) && (src[(pos - 1)] == '-')) {
      int next2 = readIntString(src, &pos);
//      std::cerr << "case 1: " << next << " " << next2 << std::endl;
      for (unsigned int i = next; i <= next2; i++) {
        blacklist.push_back(i);
      }
    }
    else {
//      std::cerr << "case 2: " << next << std::endl;
      blacklist.push_back(next);
    }
  }
//  std::cerr << "done" << std::endl;
}

bool isBlacklisted(Blacklist blacklist, int value) {
  for (unsigned int i = 0; i < blacklist.size(); i++) {
    if (blacklist[i] == value) {
      return true;
    }
  }
  
  return false;
}

int processTile(const TGraphic& srcG, int x, int y,
                const GbVram& vram,
                GbTileId* dstId,
                vector<GbPattern>& rawTiles,
                vector<GbPattern>& rawTilesFlipX,
                vector<GbPattern>& rawTilesFlipY,
                vector<GbPattern>& rawTilesFlipXY,
                const vector<int>& backgroundedTileIndices,
                int forcePaletteLine = -1,
                bool disableFlipping = false) {
  lastProcessTileFoundBackgroundedTile = false;
  // Get target graphic
//  TGraphic tileG(GbPattern::w, GbPattern::h);
//  tileG.copy(srcG,
//             TRect(0, 0, 0, 0),
//             TRect(x, y, 0, 0));
  
  int paletteNum = 0;
  GbPattern pattern;
  // If palette forcing is on, use the specified palette
/*  if (forcePaletteLine != -1) {
    paletteNum = forcePaletteLine;
    int result = pattern.fromColorGraphicGG(
                             srcG, vram.getPalette(forcePaletteLine),
                             x, y);
    if (result != 0) paletteNum = GbVram::numPaletteLines;
  }
  else {
    // Find a palette that matches this part of the image.
    // If none matches, we can't proceed.
    for ( ; paletteNum < GbVram::numPaletteLines; paletteNum++) {
      int result = pattern.fromColorGraphicGG(
                               srcG, vram.getPalette(paletteNum),
                               x, y);
      if (result == 0) break;
    }
  }
  
  if (paletteNum >= GbVram::numPaletteLines) return -1;
  
  dstId->palette = paletteNum;*/
  
  pattern.fromGraphic(srcG, NULL, x, y);
  
  // Determine if target graphic matches any existing tile.
  // If so, we don't need to add a new tile.
  // We must also account for possible horizontal/vertical flipping.
  bool foundMatch = false;
  
  // Check backgrounded tiles.
  // For now, we don't bother checking for flipping (this is intended
  // for things like text where it doesn't matter)
  for (int i = 0; i < backgroundedTileIndices.size(); i++) {
    int index = backgroundedTileIndices[i];
    const GbPattern& checkPattern
      = vram.getPattern(index);
    
    if (pattern == checkPattern) {
      dstId->pattern = index;
      foundMatch = true;
      lastProcessTileFoundBackgroundedTile = true;
      break;
    }
  }
  
  if (foundMatch) goto done;
  
  for (int i = 0; i < rawTiles.size(); i++) {
    if (pattern == rawTiles[i]) {
      dstId->pattern = i;
      
      foundMatch = true;
      break;
    }
/*    else if (!disableFlipping
              && (pattern == rawTilesFlipX[i])) {
      
      dstId->pattern = i;
      dstId->hflip = true;
      
      foundMatch = true;
      break;
    }
    else if (!disableFlipping
              && (pattern == rawTilesFlipY[i])) {
      
      dstId->pattern = i;
      dstId->vflip = true;
      
      foundMatch = true;
      break;
    }
    else if (!disableFlipping
              && (pattern == rawTilesFlipXY[i])) {
      
      dstId->pattern = i;
      dstId->hflip = true;
      dstId->vflip = true;
      
      foundMatch = true;
      break;
    }*/
  }

done:

  // if we found a match, we're done
  if (foundMatch) {
//    cout << dstId->pattern << endl;
    return 0;
  }
  
  // otherwise, add a new tile
  
  rawTiles.push_back(pattern);
  
  GbPattern flipcopy = pattern;
  flipcopy.flipH();
  rawTilesFlipX.push_back(flipcopy);
  
  pattern.flipV();
  rawTilesFlipY.push_back(pattern);
  
  flipcopy.flipV();
  rawTilesFlipXY.push_back(flipcopy);
  
  dstId->pattern = rawTiles.size() - 1;
  
  return 0;
}

int main(int argc, char* argv[]) {
  
  
  // Input:
  // * output filename for graphics
  //   (tilemaps assumed from input names)
  // * raw graphic(s)
  // * target offset in VRAM of tilemapped data
  // * optional output prefix
  // * palette
  //   (don't think we need this on a per-file basis?)
  
  if (argc < 2) {
    cout << "Game Boy tilemap generator" << endl;
    cout << "Usage: " << argv[0] << " <scriptfile>" << endl;
    
    return 0;
  }
  
  TIniFile script = TIniFile(string(argv[1]));
  
  if (!script.hasSection("Properties")) {
    cerr << "Error: Script has no 'Properties' section" << endl;
    return 1;
  }
  
  string paletteName, destName;
  int loadAddr = 0;
  int minTiles = 0;
  int maxTiles = -1;
  Blacklist blacklist;
  
  // Mandatory fields
  
/*  if (!script.hasKey("Properties", "palette")) {
    cerr << "Error: Properties.palette is undefined" << endl;
    return 1;
  }
  paletteName = script.valueOfKey("Properties", "palette");*/
  
  if (!script.hasKey("Properties", "dest")) {
    cerr << "Error: Properties.dest is undefined" << endl;
    return 1;
  }
  destName = script.valueOfKey("Properties", "dest");
  
  // Optional fields
  
  if (script.hasKey("Properties", "loadAddr")) {
    loadAddr = TStringConversion::stringToInt(script.valueOfKey(
      "Properties", "loadAddr"));
  }
  
  if (script.hasKey("Properties", "minTiles")) {
    minTiles = TStringConversion::stringToInt(script.valueOfKey(
      "Properties", "minTiles"));
  }
  
  if (script.hasKey("Properties", "maxTiles")) {
    maxTiles = TStringConversion::stringToInt(script.valueOfKey(
      "Properties", "maxTiles"));
  }
  
  if (script.hasKey("Properties", "blacklist")) {
    string blacklistStr = script.valueOfKey("Properties", "blacklist");
    readBlacklist(blacklist, blacklistStr);
  }
  
//  for (int i = 0; i < blacklist.size(); i++) {
//    cout << blacklist[i] << endl;
//  }
  
  // get palette
//  GbPalette palette;
  GbVram vram;
/*  {
    TIfstream ifs(paletteName.c_str(), ios_base::binary);
    GbPalette pal;
    pal.readGG(ifs);
    vram.setTilePalette(pal);
    
    if (ifs.remaining() > 0) {
      pal.readGG(ifs);
      vram.setSpritePalette(pal);
    }
  }*/
  
  // If any tiles have been backgrounded, load them to VRAM at the
  // specified positions
  vector<int> backgroundedTileIndices;
  for (SectionKeysMap::const_iterator it = script.cbegin();
       it != script.cend();
       ++it) {
    // iterate over all sections beginning with "Background"
    string cmpstr = "Background";
    if (it->first.substr(0, cmpstr.size()).compare(cmpstr) != 0) continue;
    string sectionName = it->first;
    
    if (!script.hasKey(sectionName, "source")) {
      cerr << "Error: " << sectionName << ".source is undefined" << endl;
      return 1;
    }
    string sourceStr = script.valueOfKey(sectionName, "source");
    
    if (!script.hasKey(sectionName, "loadaddr")) {
      cerr << "Error: " << sectionName << ".loadaddr is undefined" << endl;
      return 1;
    }
    int loadaddr = TStringConversion::stringToInt(
      script.valueOfKey(sectionName, "loadaddr"));
    
    TIfstream ifs(sourceStr.c_str(), ios_base::binary);
    while (ifs.remaining() > 0) {
      GbPattern pattern;
      pattern.read(ifs);
      backgroundedTileIndices.push_back(loadaddr);
      vram.setPattern(loadaddr, pattern);
      ++loadaddr;
    }
    
  }
  
  // 1. go through all source images and analyze for matching tiles
  // 2. create per-image tilemap corresponding to raw tile indices
  // 3. map raw tile indices to actual tile positions (accounting for
  //    blacklist, etc.)
  // 4. generate final tilemaps by mapping raw indices to final positions
  
//  vector<MdTilemap> rawTilemaps;
  map<string, GbTilemapExt> rawTilemaps;
  vector<GbPattern> rawTiles;
  vector<GbPattern> rawTilesFlipX;
  vector<GbPattern> rawTilesFlipY;
  vector<GbPattern> rawTilesFlipXY;
  
  for (SectionKeysMap::const_iterator it = script.cbegin();
       it != script.cend();
       ++it) {
    // iterate over all sections beginning with "Tilemap"
    string cmpstr = "Tilemap";
    if (it->first.substr(0, cmpstr.size()).compare(cmpstr) != 0) continue;
    string sectionName = it->first;
    
    string sourceStr, copyPriorityMapStr;
    int blanketPriority = 0;
    bool hasCopyPriorityMap = false;
    int forcePaletteLine = -1;
    bool noflip = false;
    
    // mandatory fields
    
    if (!script.hasKey(sectionName, "source")) {
      cerr << "Error: " << sectionName << ".source is undefined" << endl;
      return 1;
    }
    sourceStr = script.valueOfKey(sectionName, "source");
    
    // optional fields
    
    if (script.hasKey(sectionName, "copyPriorityMap")) {
      copyPriorityMapStr = script.valueOfKey(sectionName, "copyPriorityMap");
      hasCopyPriorityMap = true;
      
      // read copy map
      // ...
    }
    
    if (script.hasKey(sectionName, "priority")) {
      blanketPriority = TStringConversion::stringToInt(
        script.valueOfKey(sectionName, "priority"));
    }
    
    if (script.hasKey(sectionName, "palette")) {
      forcePaletteLine = TStringConversion::stringToInt(
        script.valueOfKey(sectionName, "palette"));
    }
    
    if (script.hasKey(sectionName, "noflip")) {
      noflip = (TStringConversion::stringToInt(
        script.valueOfKey(sectionName, "noflip")) != 0);
    }
    
    // halfwidth tilemaps cannot use flipping
    if (script.hasKey(sectionName, "halfwidth")) {
      bool halfwidth = false;
      halfwidth = (TStringConversion::stringToInt(
        script.valueOfKey(sectionName, "halfwidth")) != 0);
      if (halfwidth) noflip = true;
    }
    
    // get source graphic
    TGraphic srcG;
    TPngConversion::RGBAPngToGraphic(sourceStr, srcG);
    
    // infer tilemap dimensions from source size
    int tileW = srcG.w() / GbPattern::w;
    int tileH = srcG.h() / GbPattern::h;
    
    GbTilemapExt tilemap;
    tilemap.resize(tileW, tileH);
    
//    cout << tileW << " " << tileH << endl;

    for (int j = 0; j < tilemap.tileIds.h(); j++) {
      for (int i = 0; i < tilemap.tileIds.w(); i++) {
        GbTileId& tileId = tilemap.tileIds.data(i, j);
        tileId.priority = blanketPriority;
        tileId.hflip = false;
        tileId.vflip = false;
        // copy from priority map if needed
        // ...
      
        int result = processTile(srcG, i * GbPattern::w, j * GbPattern::h,
                                 vram,
                                 &tileId,
                                 rawTiles, rawTilesFlipX, rawTilesFlipY,
                                 rawTilesFlipXY,
                                 backgroundedTileIndices,
                                 forcePaletteLine,
                                 noflip);
        
        if (result != 0) {
          cerr << "Error in " << sectionName
            << ": failed processing tile (" << i << ", " << j << ")" << endl;
          return 2;
        }
        
        if (lastProcessTileFoundBackgroundedTile) {
          tilemap.isRaw.data(i, j) = true;
        }
        else {
          tilemap.isRaw.data(i, j) = false;
        }
      }
    }
    
    rawTilemaps[sectionName] = tilemap;
  }
  
//  cout << rawTiles.size() << endl;

  // Produce the final arrangement of tiles
  
  map<int, GbPattern> outputTiles;
  map<int, int> rawToOutputMap;
  int outputTileNum = 0;
  for (int i = 0; i < rawTiles.size(); i++) {
    // Skip blacklisted content
    while (isBlacklisted(blacklist, outputTileNum + loadAddr)) {
      outputTiles[outputTileNum] = GbPattern();
      ++outputTileNum;
    }
    
    outputTiles[outputTileNum] = rawTiles[i];
    rawToOutputMap[i] = outputTileNum;
    ++outputTileNum;
  }
  
  // create identity mapping for backgrounded tiles
  for (int i = 0; i < backgroundedTileIndices.size(); i++) {
//    rawToOutputMap[backgroundedTileIndices[i]] = backgroundedTileIndices[i];
    // HACK: oops i forgot to properly account for load addresses
    rawToOutputMap[backgroundedTileIndices[i]]
      = backgroundedTileIndices[i];
  }
  
  // Give an error if tile limit exceeded
  if (outputTiles.size() > maxTiles) {
    cerr << "Error: Tile limit exceeded (limit is "
      << maxTiles << "; generated "
      << outputTiles.size() << ")" << endl;
    return -3;
  }
  
  // Write tile data
  {
    TOfstream ofs(destName.c_str(), ios_base::binary);
//    TBufStream buffer(GbPattern::size);
    for (map<int, GbPattern>::const_iterator it = outputTiles.cbegin();
         it != outputTiles.cend();
         ++it) {
      it->second.write(ofs);
//      ofs.write(buffer.data().data(), GbPattern::size);
    }
    
    // pad with extra tiles to meet minimum length
//    memset((char*)buffer.data().data(), 0, GbPattern::size);
    int padTiles = minTiles - outputTiles.size();
    GbPattern pattern;
    for (int i = 0; i < padTiles; i++) {
//      ofs.write(buffer.data().data(), GbPattern::size);
      pattern.write(ofs);
    }
  }
  
  // Update tilemaps and write to destinations
  
  for (SectionKeysMap::const_iterator it = script.cbegin();
       it != script.cend();
       ++it) {
    // iterate over all sections beginning with "Tilemap"
    string cmpstr = "Tilemap";
    if (it->first.substr(0, cmpstr.size()).compare(cmpstr) != 0) continue;
    string sectionName = it->first;
    
    if (!script.hasKey(sectionName, "dest")) {
      cerr << "Error: " << sectionName << ".dest is undefined" << endl;
      return 1;
    }
    string destStr = script.valueOfKey(sectionName, "dest");
    
    bool halfwidth = false;
    if (script.hasKey(sectionName, "halfwidth")) {
      halfwidth = (TStringConversion::stringToInt(
        script.valueOfKey(sectionName, "halfwidth")) != 0);
    }
    
    GbTilemapExt& tilemap = rawTilemaps[sectionName];
    
    TOfstream ofs(destStr.c_str(), ios_base::binary);
//    TBufStream ofs(tilemap.tileIds.h() * tilemap.tileIds.w() * 2);
    
    for (int j = 0; j < tilemap.tileIds.h(); j++) {
      for (int i = 0; i < tilemap.tileIds.w(); i++) {
        GbTileId& id = tilemap.tileIds.data(i, j);
        if (!tilemap.isRaw.data(i, j))
          id.pattern = rawToOutputMap[id.pattern] + loadAddr;
        char buffer[GbTileId::size];
        id.write(buffer);
    
        // if halfwidth, low byte only
        if (halfwidth) {
          ofs.write(buffer, GbTileId::size / 2);
        }
        else {
          ofs.write(buffer, GbTileId::size);
        }
      }
    }
  }
  
  return 0;
}
