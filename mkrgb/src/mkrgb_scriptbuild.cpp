#include "mkrgb/MkrGbScriptReader.h"
#include "mkrgb/MkrGbLineWrapper.h"
//#include "mkrgb/MkrGbTextScript.h"
#include "util/TBufStream.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TGraphic.h"
#include "util/TStringConversion.h"
#include "util/TFileManip.h"
#include "util/TPngConversion.h"
#include "util/TFreeSpace.h"
#include <cctype>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;
using namespace BlackT;
using namespace Gb;

const static int sectorSize = 0x800;

const static int textCharsStart = 0x20;
const static int textCharsEnd = textCharsStart + 0x60;
const static int textEncodingMax = 0x100;
const static int maxDictionarySymbols = textEncodingMax - textCharsEnd;

const static int fontEmphToggleOp = 0x7F;

const static int op_jump = 0xF9;
const static int op_call = 0xF8;
//const static int op_ret = 0xF7;
const static int op_nothing = 0xF7;
const static int op_terminator = 0x7F;

TThingyTable table;

string as2bHex(int num) {
  string str = TStringConversion::intToString(num,
                  TStringConversion::baseHex).substr(2, string::npos);
  while (str.size() < 2) str = string("0") + str;
  
//  return "<$" + str + ">";
  return str;
}

string as2bHexPrefix(int num) {
  return "$" + as2bHex(num) + "";
}

std::string getNumStr(int num) {
  std::string str = TStringConversion::intToString(num);
  while (str.size() < 2) str = string("0") + str;
  return str;
}

std::string getHexByteNumStr(int num) {
  std::string str = TStringConversion::intToString(num,
    TStringConversion::baseHex).substr(2, string::npos);
  while (str.size() < 2) str = string("0") + str;
  return string("$") + str;
}

std::string getHexWordNumStr(int num) {
  std::string str = TStringConversion::intToString(num,
    TStringConversion::baseHex).substr(2, string::npos);
  while (str.size() < 4) str = string("0") + str;
  return string("$") + str;
}
                      

void binToDcb(TStream& ifs, std::ostream& ofs) {
  int constsPerLine = 16;
  
  while (true) {
    if (ifs.eof()) break;
    
    ofs << "  .db ";
    
    for (int i = 0; i < constsPerLine; i++) {
      if (ifs.eof()) break;
      
      TByte next = ifs.get();
      ofs << as2bHexPrefix(next);
      if (!ifs.eof() && (i != constsPerLine - 1)) ofs << ",";
    }
    
    ofs << std::endl;
  }
}




typedef std::map<std::string, int> UseCountTable;
//typedef std::map<std::string, double> EfficiencyTable;
typedef std::map<double, std::string> EfficiencyTable;

bool isCompressible(std::string& str) {
  for (int i = 0; i < str.size(); i++) {
    if ((unsigned char)str[i] < textCharsStart) return false;
    if ((unsigned char)str[i] >= textCharsEnd) return false;
    if ((unsigned char)str[i] == fontEmphToggleOp) return false;
  }
  
  return true;
}

void addStringToUseCountTable(std::string& input,
                        UseCountTable& useCountTable,
                        int minLength, int maxLength) {
  int total = input.size() - minLength;
  if (total <= 0) return;
  
  for (int i = 0; i < total; ) {
    int basePos = i;
    for (int j = minLength; j < maxLength; j++) {
      int length = j;
      if (basePos + length >= input.size()) break;
      
      std::string str = input.substr(basePos, length);
      
      // HACK: avoid analyzing parameters of control sequences
      // the ops themselves are already ignored in the isCompressible check;
      // we just check when an op enters into the first byte of the string,
      // then advance the check position so the parameter byte will
      // never be considered
/*      if ((str.size() > 0) && ((unsigned char)str[0] < textCharsStart)) {
        unsigned char value = str[0];
        if ((value == 0x02) // "L"
            || (value == 0x05) // "P"
            || (value == 0x06)) { // "W"
          // skip the argument byte
          i += 1;
        }
        break;
      }*/
      // FIXME for project that actually uses compression
/*      if (str.size() > 0) {
        unsigned char value = str[0];
        if (MkrGbTextScript::isOp(value)) {
          // skip the arguments
          i += MkrGbTextScript::getOpArgsSize(value);
          break;
        }
      }*/
      
      if (!isCompressible(str)) break;
      
      ++(useCountTable[str]);
    }
    
    // skip literal arguments to ops
/*    if ((unsigned char)input[i] < textCharsStart) {
      ++i;
      int opSize = numOpParamWords((unsigned char)input[i]);
      i += opSize;
    }
    else {
      ++i;
    } */
    ++i;
  }
}

void addRegionsToUseCountTable(MkrGbScriptReader::NameToRegionMap& input,
                        UseCountTable& useCountTable,
                        int minLength, int maxLength) {
  for (MkrGbScriptReader::NameToRegionMap::iterator it = input.begin();
       it != input.end();
       ++it) {
    MkrGbScriptReader::ResultCollection& results = it->second.strings;
    for (MkrGbScriptReader::ResultCollection::iterator jt = results.begin();
         jt != results.end();
         ++jt) {
//      std::cerr << jt->srcOffset << std::endl;
      if (jt->isLiteral) continue;
      if (jt->isNotCompressible) continue;
      
      addStringToUseCountTable(jt->str, useCountTable,
                               minLength, maxLength);
    }
  }
}

void buildEfficiencyTable(UseCountTable& useCountTable,
                        EfficiencyTable& efficiencyTable) {
  for (UseCountTable::iterator it = useCountTable.begin();
       it != useCountTable.end();
       ++it) {
    std::string str = it->first;
    // penalize by 1 byte (length of the dictionary code)
    double strLen = str.size() - 1;
    double uses = it->second;
//    efficiencyTable[str] = strLen / uses;
    
    efficiencyTable[strLen / uses] = str;
  }
}

void applyDictionaryEntry(std::string entry,
                          MkrGbScriptReader::NameToRegionMap& input,
                          std::string replacement) {
  for (MkrGbScriptReader::NameToRegionMap::iterator it = input.begin();
       it != input.end();
       ++it) {
    MkrGbScriptReader::ResultCollection& results = it->second.strings;
    int index = -1;
    for (MkrGbScriptReader::ResultCollection::iterator jt = results.begin();
         jt != results.end();
         ++jt) {
      ++index;
      
      if (jt->isNotCompressible) continue;
      
      std::string str = jt->str;
      if (str.size() < entry.size()) continue;
      
      std::string newStr;
      int i;
      for (i = 0; i < str.size() - entry.size(); ) {
        // FIXME for project that actually uses compression
/*        if (MkrGbTextScript::isOp((unsigned char)str[i])) {
          int numParams = MkrGbTextScript::getOpArgsSize((unsigned char)str[i]);
          newStr += str[i++];
          for (int j = 0; j < numParams; j++) {
            newStr += str[i + j];
          }
          i += numParams;
          
          continue;
        }*/
        
        if (entry.compare(str.substr(i, entry.size())) == 0) {
          newStr += replacement;
          i += entry.size();
        }
        else {
          newStr += str[i];
          ++i;
        }
      }
      
      while (i < str.size()) newStr += str[i++];
      
      jt->str = newStr;
    }
  }
}

void generateCompressionDictionary(
    MkrGbScriptReader::NameToRegionMap& results,
    std::string outputDictFileName) {
  TBufStream dictOfs;
  for (int i = 0; i < maxDictionarySymbols; i++) {
//    cerr << i << endl;
    UseCountTable useCountTable;
    addRegionsToUseCountTable(results, useCountTable, 2, 3);
    EfficiencyTable efficiencyTable;
    buildEfficiencyTable(useCountTable, efficiencyTable);
    
//    std::cout << efficiencyTable.begin()->first << std::endl;
    
    // if no compressions are possible, give up
    if (efficiencyTable.empty()) break;  
    
    int symbol = i + textCharsEnd;
    applyDictionaryEntry(efficiencyTable.begin()->second,
                         results,
                         std::string() + (char)symbol);
    
    // debug
/*    TBufStream temp;
    temp.writeString(efficiencyTable.begin()->second);
    temp.seek(0);
//    binToDcb(temp, cout);
    std::cout << "\"";
    while (!temp.eof()) {
      std::cout << table.getEntry(temp.get());
    }
    std::cout << "\"" << std::endl; */
    
    dictOfs.writeString(efficiencyTable.begin()->second);
  }
  
//  dictOfs.save((outPrefix + "dictionary.bin").c_str());
  dictOfs.save(outputDictFileName.c_str());
}

// merge a set of NameToRegionMaps into a single NameToRegionMap
void mergeResultMaps(
    std::vector<MkrGbScriptReader::NameToRegionMap*>& allSrcPtrs,
    MkrGbScriptReader::NameToRegionMap& dst) {
  int targetOutputId = 0;
  for (std::vector<MkrGbScriptReader::NameToRegionMap*>::iterator it
        = allSrcPtrs.begin();
       it != allSrcPtrs.end();
       ++it) {
    MkrGbScriptReader::NameToRegionMap& src = **it;
    for (MkrGbScriptReader::NameToRegionMap::iterator jt = src.begin();
         jt != src.end();
         ++jt) {
      dst[TStringConversion::intToString(targetOutputId++)] = jt->second;
    }
  }
}

// undo the effect of mergeResultMaps(), applying any changes made to
// the merged maps back to the separate originals
void unmergeResultMaps(
    MkrGbScriptReader::NameToRegionMap& src,
    std::vector<MkrGbScriptReader::NameToRegionMap*>& allSrcPtrs) {
  int targetInputId = 0;
  for (std::vector<MkrGbScriptReader::NameToRegionMap*>::iterator it
        = allSrcPtrs.begin();
       it != allSrcPtrs.end();
       ++it) {
    MkrGbScriptReader::NameToRegionMap& dst = **it;
    for (MkrGbScriptReader::NameToRegionMap::iterator jt = dst.begin();
         jt != dst.end();
         ++jt) {
      jt->second = src[TStringConversion::intToString(targetInputId++)];
    }
  }
}

void exportGenericRegion(MkrGbScriptReader::ResultCollection& results,
                         std::string prefix) {
  for (MkrGbScriptReader::ResultCollection::iterator it = results.begin();
       it != results.end();
       ++it) {
    if (it->str.size() <= 0) continue;
    
    MkrGbScriptReader::ResultString str = *it;
    
    std::string outName = prefix + str.id + ".bin";
    TFileManip::createDirectoryForFile(outName);
    
    TBufStream ofs;
    ofs.writeString(str.str);
    ofs.save(outName.c_str());
  }
}

void exportGenericRegionMap(MkrGbScriptReader::NameToRegionMap& results,
                         std::string prefix) {
  for (auto it: results) {
    exportGenericRegion(it.second.strings, prefix);
  }
}

/*void exportRegionCreditsInclude(MkrGbScriptReader::ResultRegion& results,
                         std::string regionName, std::string prefix) {
  std::string fileName = prefix + "credits_text.inc";
  TFileManip::createDirectoryForFile(fileName);
  std::ofstream ofs(fileName.c_str());
  
  for (auto it: results.freeSpace.freeSpace_) {
    int start = 0x10000 + it.first + 0x6000;
    int end = start + it.second + 0x6000 - 1;
    ofs << ".unbackground " << start << " " << end
      << std::endl;
  }
  
  for (auto& it: results.strings) {
    std::string labelName = it.id;
    std::string sectionName = "credits str "
      + it.id;
    
    ofs << ".bank 1 slot 0" << endl;
    ofs << ".section \"" << sectionName << "\" free" << endl;
    ofs << "  " << labelName << ":" << endl;
//    ofs << "  .incbin \"out/script/strings/" << it.id << ".bin\"" << endl;
//    ofs << binToDcb(it.str) << endl;
    {
      TBufStream ifs;
      ifs.writeString(it.str);
      ifs.put(0x00);
      ifs.seek(0);
      binToDcb(ifs, ofs);
    }
    ofs << ".ends" << endl;
    
    for (auto& jt: it.pointerRefs) {
      ofs << ".bank 2 slot 0" << endl;
      ofs << ".orga $"
        << TStringConversion::intToString(jt + 0xC000,
              TStringConversion::baseHex).substr(2, string::npos)
        << endl;
      ofs << ".section \"" << sectionName << " ref " << jt << "\" overwrite" << endl;
      ofs << "  .dw " << labelName << endl;
      ofs << ".ends" << endl;
    }
  }
}

void exportCreditsIncludes(MkrGbScriptReader::NameToRegionMap& results,
                         std::string prefix) {
  for (auto& it: results) {
    exportRegionCreditsInclude(it.second, it.first, prefix);
  }
}*/

void exportRegionOverwriteInclude(MkrGbScriptReader::ResultRegion& results,
                         std::string fileName,
                         bool fixedLen = false,
                         bool jumpToEnd = false) {
//  std::string fileName = prefix + "credits_text.inc";
  TFileManip::createDirectoryForFile(fileName);
  std::ofstream ofs(fileName.c_str());
  
  if (!fixedLen) {
    for (auto& it: results.strings) {
/*      if (jumpToEnd) {
        if ((it.originalSize > 9)) {
          int start = it.originalOffset + 8;
          int end = it.originalOffset + it.originalSize - 2;
          ofs << ".unbackground " << start << " " << end
            << std::endl;
        }
      else {
        if ((it.originalSize > 4)) {
          int start = it.originalOffset + 4;
          int end = it.originalOffset + it.originalSize - 1;
          ofs << ".unbackground " << start << " " << end
            << std::endl;
        }
      }*/
      if (jumpToEnd || it.propertyIsTrue("jumpToEnd")) {
        if ((it.originalSize > 5)) {
          int start = it.originalOffset + 4;
          // preserve terminator
          int end = it.originalOffset + it.originalSize - 2;
          ofs << ".unbackground " << start << " " << end
            << std::endl;
        }
      }
      else {
        if ((it.originalSize > 4)) {
          int start = it.originalOffset + 4;
          int end = it.originalOffset + it.originalSize - 1;
          ofs << ".unbackground " << start << " " << end
            << std::endl;
        }
      }
    }
  }

  ofs << std::endl;
  
  for (auto& it: results.strings) {
//    std::string labelName = it.id;
    int bankNum = it.originalOffset / 0x4000;
    int bankOffset = it.originalOffset % 0x4000;
    int endBankNum = (it.originalOffset + it.originalSize - 1) / 0x4000;
    int endBankOffset = (it.originalOffset + it.originalSize - 1) % 0x4000;
    
    std::string labelName
      = std::string("loc_")
        + TStringConversion::intToString(it.originalOffset,
            TStringConversion::baseHex);
    std::string sectionName = "overwrite str "
      + labelName;
    
    ofs << ".slot 1" << endl;
    ofs << ".section \"" << sectionName << "\" superfree" << endl;
    ofs << "  " << labelName << ":" << endl;
//    ofs << "  .incbin \"out/script/strings/" << it.id << ".bin\"" << endl;
//    ofs << binToDcb(it.str) << endl;
    {
      TBufStream ifs;
      if ((jumpToEnd || it.propertyIsTrue("jumpToEnd")) && !fixedLen) {
        // omit trailing terminator to allow for jump command
        // at end of script, which jumps back to original script
        // terminator so that execution continues in the correct place
        ifs.writeString(it.str.substr(0, it.str.size() - 1));
      }
      else {
        ifs.writeString(it.str);
      }
      
//      ifs.put(0x00);
      if (fixedLen) {
//        ifs.put(op_ret);
        ifs.put(op_terminator);
      }
      
      ifs.seek(0);
      binToDcb(ifs, ofs);
      
      if (jumpToEnd || it.propertyIsTrue("jumpToEnd")) {
        ofs << "  .db " << getHexByteNumStr(op_jump) << endl;
        ofs << "  .dw " << getHexWordNumStr(endBankOffset + 0x4000) << endl;
        ofs << "  .db " << getHexByteNumStr(endBankNum) << endl;
      }
    }
    ofs << ".ends" << endl;
    ofs << endl;
    
    if (it.originalSize > 4) {
      ofs << ".bank " << bankNum << " slot 1" << endl;
      ofs << ".org " << getHexWordNumStr(bankOffset) << endl;
      ofs << ".section \"" << sectionName << " ref\" overwrite" << endl;
      
      if (fixedLen) {
        ofs << "  .db " << as2bHexPrefix(op_call) << endl;
        ofs << "  .dw " << labelName << endl;
        ofs << "  .db :" << labelName << endl;
        
        TBufStream temp;
        for (int i = 0; i < it.originalSize - 4; i++) {
          temp.put(op_nothing);
        }
        if (temp.size() > 0) {
          temp.seek(0);
          binToDcb(temp, ofs);
        }
      }
      else {
        ofs << "  .db " << as2bHexPrefix(op_jump) << endl;
        ofs << "  .dw " << labelName << endl;
        ofs << "  .db :" << labelName << endl;
      }
      
      ofs << ".ends" << endl;
      ofs << endl;
    }
    
    
/*    for (auto& jt: it.pointerRefs) {
      ofs << ".bank 2 slot 0" << endl;
      ofs << ".orga $"
        << TStringConversion::intToString(jt + 0xC000,
              TStringConversion::baseHex).substr(2, string::npos)
        << endl;
      ofs << ".section \"" << sectionName << " ref " << jt << "\" overwrite" << endl;
      ofs << "  .dw " << labelName << endl;
      ofs << ".ends" << endl;
    }*/
  }
}

void exportOverwriteIncludes(MkrGbScriptReader::NameToRegionMap& results,
                         std::string filename,
                         bool fixedLen = false,
                         bool jumpToEnd = false) {
  for (auto& it: results) {
    exportRegionOverwriteInclude(it.second, filename, fixedLen, jumpToEnd);
  }
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    cout << "Magic Knight Rayearth (GB) script builder" << endl;
    cout << "Usage: " << argv[0] << " [inprefix] [outprefix]"
      << endl;
    return 0;
  }
  
//  string infile(argv[1]);
  string inPrefix(argv[1]);
  string outPrefix(argv[2]);

//  table.readUtf8("table/mkrgb_en.tbl");
//  tableScene.readUtf8("table/mkrgb_scenes_en.tbl");
  table.readUtf8("table/mkrgb_en.tbl");
  
  //=====
  // read script
  //=====
  
  MkrGbScriptReader::NameToRegionMap scriptResults;
  {
    TBufStream ifs;
    ifs.open((inPrefix + "script.txt").c_str());
    MkrGbScriptReader(ifs, scriptResults, table)();
  }
  
  MkrGbScriptReader::NameToRegionMap scriptResultsCredits;
  {
    TBufStream ifs;
    ifs.open((inPrefix + "script_credits.txt").c_str());
    MkrGbScriptReader(ifs, scriptResultsCredits, table)();
  }
  
  MkrGbScriptReader::NameToRegionMap scriptResultsFixedlen;
  {
    TBufStream ifs;
    ifs.open((inPrefix + "script_fixedlen.txt").c_str());
    MkrGbScriptReader(ifs, scriptResultsFixedlen, table)();
  }
  
  MkrGbScriptReader::NameToRegionMap scriptResultsMisc;
  {
    TBufStream ifs;
    ifs.open((inPrefix + "script_misc.txt").c_str());
    MkrGbScriptReader(ifs, scriptResultsMisc, table)();
  }
  
  MkrGbScriptReader::NameToRegionMap scriptResultsPlaces;
  {
    TBufStream ifs;
    ifs.open((inPrefix + "script_places.txt").c_str());
    MkrGbScriptReader(ifs, scriptResultsPlaces, table)();
  }
  
//  generateCompressionDictionary(
//    scriptResults, outPrefix + "script_dictionary.bin");
  
  //=====
  // compress
  //=====
  
/*  {
    MkrGbScriptReader::NameToRegionMap allStrings;
    
    std::vector<MkrGbScriptReader::NameToRegionMap*> allSrcPtrs;
    allSrcPtrs.push_back(&scriptResults);
//    allSrcPtrs.push_back(&battleResults);
    
    // merge everything into one giant map for compression
    mergeResultMaps(allSrcPtrs, allStrings);
    
    // compress
    generateCompressionDictionary(
      allStrings, outPrefix + "script_dictionary.bin");
    
    // restore results from merge back to individual containers
    unmergeResultMaps(allStrings, allSrcPtrs);
  }*/
  
  //=====
  // script
  //=====
  
  exportOverwriteIncludes(scriptResults,
    "asm/gen/script.inc", false, true);
  exportOverwriteIncludes(scriptResultsCredits,
    "asm/gen/script_credits.inc", true);
  exportOverwriteIncludes(scriptResultsFixedlen,
    "asm/gen/script_fixedlen.inc", true);
  exportOverwriteIncludes(scriptResultsMisc,
    "asm/gen/script_misc.inc", false);
  exportOverwriteIncludes(scriptResultsPlaces,
    "asm/gen/script_places.inc", true);
  
  //=====
  // export generic/hardcoded strings
  //=====
  
  // TODO
  
/*  exportGenericRegionMap(advSceneResults, "out/script/strings/");
  exportGenericRegionMap(visualResults, "out/script/strings/");
  exportGenericRegionMap(miscResults, "out/script/strings/");
  exportGenericRegionMap(t8x8Results, "out/script/strings/");
  exportGenericRegionMap(creditsResults, "out/script/strings/");
//  exportGenericRegionMap(creditsTextResults, "out/script/strings/");*/
  
  return 0;
}
