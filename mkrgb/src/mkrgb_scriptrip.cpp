#include "util/TThingyTable.h"
#include "util/TStringConversion.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TBufStream.h"
#include "util/TCsv.h"
#include "exception/TGenericException.h"
#include <string>
#include <map>
#include <sstream>
#include <fstream>
#include <iostream>

using namespace std;
using namespace BlackT;
//using namespace Gb;

const static int op_op7A      = 0x7A;
const static int op_op7B      = 0x7B;
const static int op_clear     = 0x7C;
const static int op_br        = 0x7D;
const static int op_wait      = 0x7E;
const static int op_end       = 0x7F;
const static int op_delay     = 0xFA;
const static int op_opFB      = 0xFB;
const static int op_sound     = 0xFC;
const static int op_opFD      = 0xFD;
const static int op_opFE      = 0xFE;
const static int op_portrait  = 0xFF;

typedef std::map<std::string, std::string> NametagToTranslationMap;
static NametagToTranslationMap nametagMap;

bool isOpId(int op) {
  if (((op >= 0x7A) && (op <= 0x7F))
      || ((op >= 0xFA) && (op <= 0xFF))) {
    return true;
  }
  
  return false;
}

bool isSharedOp(int op) {
  switch (op) {
  case op_op7A:
  case op_op7B:
  case op_clear:
//  case op_br:
  case op_wait:
  case op_end:
//  case op_delay:
  case op_opFB:
  case op_sound:
  case op_opFD:
  case op_opFE:
  case op_portrait:
    return true;
  default:
    break;
  }
  
  return false;
}

int numOpParamBytes(int op) {
  switch (op) {
  case op_delay:
  case op_opFB:
  case op_sound:
  case op_opFD:
  case op_opFE:
  case op_portrait:
    return 1;
  default:
    break;
  }
  
  return 0;
}

int numOpPreLines(int op) {
  switch (op) {
  case op_wait:
  case op_clear:
  case op_end:
    return 1;
    break;
  default:
    break;
  }
  
  return 0;
}

int numOpPostLines(int op) {
  switch (op) {
  case op_br:
    return 1;
    break;
  case op_wait:
  case op_end:
  case op_clear:
    return 2;
    break;
  default:
    break;
  }
  
  if (isSharedOp(op)) return 1;
  
  return 0;
}

string as2bHex(int num) {
  string str = TStringConversion::intToString(num,
                  TStringConversion::baseHex).substr(2, string::npos);
  while (str.size() < 2) str = string("0") + str;
  
  return "<$" + str + ">";
}

bool scriptStringExistsAtOffset(TStream& ifs, const TThingyTable& table,
                    int offset) {
  ifs.seek(offset);
  
  while (!ifs.eof()) {
    TThingyTable::MatchResult result = table.matchId(ifs);
    if (result.id == -1) {
      return false;
    }
    
    int id = result.id;
    bool isOp = isOpId(id);
      
    if (isOp) {
      if (id == op_end) break;
      
      int numParamBytes = numOpParamBytes(id);
      for (int i = 0; i < numParamBytes; i++) {
        ifs.get();
      }
    }
  }
  
  if (ifs.eof()) {
    return false;
  }
  
  return true;
}

void addComment(std::ostream& ofs, string comment) {
  ofs << "//===========================================================" << endl;
  ofs << "// " << comment << endl;
  ofs << "//===========================================================" << endl;
  ofs << endl;
}

void doNametagCheckAndUpdate(std::string srcLine,
                          bool& nametagPending,
                          std::string& pendingNametagContent) {
/*  bool found = false;
//  for (int i = 0; i < srcLine.size(); i++) {
//    
//  }
  
  if (!found) {
//    nametagPending = false;
//    pendingNametagContent = "";
    return;
  } */
  
  NametagToTranslationMap::iterator findIt = nametagMap.find(srcLine);
  if (findIt == nametagMap.end()) {
//    std::cout << srcLine << std::endl;
    nametagMap[srcLine] = "[]";
    findIt = nametagMap.find(srcLine);
  }
  
  nametagPending = true;
  pendingNametagContent = findIt->second;
}

void dumpString(TStream& ifs, std::ostream& ofs, const TThingyTable& table,
              int offset, int sizeLimit = -1,
              string comment = "") {
  ifs.seek(offset);
  
  std::ostringstream oss_final;
  std::ostringstream oss_textline;
  
  if (comment.size() > 0)
    oss_final << "// " << comment << endl;
  
  bool atLineStart = true;
  bool lastWasBr = false;
  bool nametagPending = false;
  std::string pendingNametagContent;
  int charsOnLine = 0;
  while (!ifs.eof()) {
    TThingyTable::MatchResult result = table.matchId(ifs);
    if (result.id == -1) {
      throw TGenericException(T_SRCANDLINE,
                              "dumpScript()",
                              string("At file offset ")
                                + TStringConversion::intToString(
                                    ifs.tell(),
                                    TStringConversion::baseHex)
                                + ": could not match character from table");
    }
    
    int id = result.id;
    
    string resultStr = table.getEntry(result.id);
    bool isOp = isOpId(id);
//                  || (id == op_waitend)
    
    if (isOp) {
      bool shared = isSharedOp(id);
      
      // HACK: if line is otherwise empty and this is a delay command,
      // treat as shared op
      bool specialSharedHack = false;
      if ((id == op_delay)
          && (atLineStart)) {
        shared = true;
        specialSharedHack = true;
      }
      
      std::ostringstream* targetOss = NULL;
      if (shared) {
        targetOss = &oss_final;
        
        // empty comment line buffer
        if (oss_textline.str().size() > 0) {
          oss_final << "// " << oss_textline.str();
          
//          std::cout << oss_final.str() << std::endl;
//          char c;
//          cin >> c;
          
          // print nametag if waiting for one
          oss_final << std::endl;
          if (nametagPending) {
            oss_final << pendingNametagContent << std::endl;
            nametagPending = false;
          }
          oss_final << std::endl;
          
          oss_textline.str("");
          atLineStart = true;
        }
      }
      else {
        targetOss = &oss_textline;
      }
      
      //===========================================
      // output pre-linebreaks
      //===========================================
      
      int numPreLines = numOpPreLines(id);
      
      // HACK
      if (specialSharedHack) {
        numPreLines = 1;
      }
      
      if ((!atLineStart || (atLineStart && lastWasBr))
          && (numPreLines > 0)) {
        if (oss_textline.str().size() > 0) {
          oss_final << "// " << oss_textline.str();
          oss_textline.str("");
        }
        
        for (int i = 0; i < numPreLines; i++) {
          oss_final << std::endl;
        }

        atLineStart = true;
      }
      
      //===========================================
      // if op is shared, output it directly to
      // the final text on its own line, separate
      // from the commented-out original
      //===========================================
      
      // non-shared op: add to commented-out original line
      *targetOss << resultStr;
      atLineStart = false;
      
      //===========================================
      // output param bytes
      //===========================================
      
      int numParamBytes = numOpParamBytes(id);
      for (int i = 0; i < numParamBytes; i++) {
        *targetOss << as2bHex(ifs.readu8());
        atLineStart = false;
      }
      
      //===========================================
      // output post-linebreaks
      //===========================================
     
      int numPostLines = numOpPostLines(id);
      
      // HACK: hack for wait/end combo
      if ((id == op_wait) && ((unsigned char)ifs.peek() == op_end)) {
        numPostLines = 1;
      }
      else if ((id == op_clear) && ((unsigned char)ifs.peek() == op_end)) {
        numPostLines = 1;
      }
      
      // HACK
      if (specialSharedHack) {
        numPostLines = 1;
      }
      
      if (numPostLines > 0) {
        if (oss_textline.str().size() > 0) {
          oss_final << "// " << oss_textline.str();
          oss_textline.str("");
        }
       
        for (int i = 0; i < numPostLines; i++) {
          oss_final << std::endl;
        }

        atLineStart = true;
      }
    }
    else {
      // account for auto-break every 7 chars
/*      ++charsOnLine;
      if ((autowrap != -1) && (charsOnLine > autowrap)) {
        oss_final << "// " << oss_textline.str();
        oss_textline.str("");
        oss_final << std::endl;
        charsOnLine = 0;
        atLineStart = true;
      } */
      
      // not an op: add to commented-out original line
      oss_textline << resultStr;
      
      // HACK: if a close-brace ("]"), check for new nametag
      if ((id == 0x3C) && (ifs.peek() == op_br)) {
        doNametagCheckAndUpdate(oss_textline.str(),
                                nametagPending,
                                pendingNametagContent);
      }
      
      atLineStart = false;
    }
    
    // check for size limit
    if (sizeLimit != -1) {
      if ((ifs.tell() - offset) >= sizeLimit) {
//        std::cerr << hex << ifs.tell() << " " << hex << offset << endl;
        oss_final << "// " << oss_textline.str();
        oss_final << std::endl << std::endl << std::endl;
        oss_textline.str("");
        break;
      }
    }
    
    // check for terminators
//    if ((id == op_end) || (id == op_wait)) {
    if ((id == op_end)) {
      break;
    }
    
    // handle line-breaking ops
    if ((id == op_br) || (id == op_wait)) charsOnLine = 0;
    
    lastWasBr = (id == op_br);
  }
  
  ofs << "//[TEXT]" << endl;
  ofs << "#STARTMSG("
      // offset
      << TStringConversion::intToString(
          offset, TStringConversion::baseHex)
      << ", "
      // size
      << TStringConversion::intToString(
          ifs.tell() - offset, TStringConversion::baseDec)
//      << ", "
//      // slot num
//      << TStringConversion::intToString(
//          slot, TStringConversion::baseDec)
      << ")" << endl << endl;
  
//  ofs << oss.str();
  ofs << oss_final.str();
  
//  ofs << endl;
  ofs << "#ENDMSG()";
  ofs << endl << endl;
}

int main(int argc, char* argv[]) {
/*  if (argc < 4) {
    cout << "Binary -> text converter, via Thingy table" << endl;
    cout << "Usage: " << argv[0] << " <thingy> <infile> <outfile>" << endl;
    cout << "The Thingy table must be in SJIS (or compatible) format."
      << endl;
    
    return 0;
  } */
  
  TThingyTable table;
  table.readSjis("table/mkrgb.tbl");
//  std::ifstream ifs(argv[2], ios_base::binary);

//  TIfstream ifs(argv[2], ios_base::binary);

  TBufStream ifs;
  ifs.open("mkrgb.gb");
  
/*  while (!ifs.eof()) {
//    int next = (unsigned char)ifs.get();
    TThingyTable::MatchResult result = thingy.matchId(ifs);
    
    if (result.id != -1) {
      ofs << thingy.getEntry(result.id);
    }
    else {
      
    }
  } */
  
  TCsv nametagsCsv;
  {
    TIfstream ifs("script_nametags.txt");
    nametagsCsv.readUtf8(ifs);
    for (int j = 0; j < nametagsCsv.numRows(); j++) {
//      cout << nametagsCsv.cell(0, j) << " " << nametagsCsv.cell(1, j);
//      cout << endl;
      nametagMap[nametagsCsv.cell(0, j)] = nametagsCsv.cell(1, j);
    }
  }

  std::ofstream ofs("script/script.txt");
  
//  dumpString(ifs, ofs, table, 0x3025B);

  //==============================
  // main script
  //==============================
  
  {
    int searchPos = 0x30000;
    ifs.seek(searchPos);
    while (!ifs.eof() && (searchPos < 0x40000)) {
      bool found = false;
      
      // marker for start of text script
      if (ifs.readu8() == 0x57) {
        if (scriptStringExistsAtOffset(ifs, table, searchPos + 1)) {
          dumpString(ifs, ofs, table, searchPos + 1);
          searchPos = ifs.tell();
          found = true;
        }
      }
      
      if (!found) ++searchPos;
      ifs.seek(searchPos);
    }
  }

  //==============================
  // misc
  //==============================

  std::ofstream ofsMisc("script/script_misc.txt");
  
  addComment(ofsMisc, "battle messages");
  ifs.seek(0x20A6);
  for (int i = 0; i < 9; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  addComment(ofsMisc, "battle menu");
  ifs.seek(0x21F9);
  for (int i = 0; i < 3; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  addComment(ofsMisc, "interface labels");
  ifs.seek(0x3B7C);
  for (int i = 0; i < 6; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  addComment(ofsMisc, "debug menu 1");
  ifs.seek(0x3F12);
  for (int i = 0; i < 3; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  addComment(ofsMisc, "debug menu 2");
  ifs.seek(0x3F2E);
  for (int i = 0; i < 9; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  addComment(ofsMisc, "debug menu 3");
  ifs.seek(0x3F6E);
  for (int i = 0; i < 4; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  addComment(ofsMisc, "debug menu 4");
  ifs.seek(0x3FBC);
  for (int i = 0; i < 2; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  addComment(ofsMisc, "save message");
  ifs.seek(0x40F1);
  for (int i = 0; i < 1; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  addComment(ofsMisc, "save/load prompt");
  ifs.seek(0x4270);
  for (int i = 0; i < 3; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  addComment(ofsMisc, "save misc");
  ifs.seek(0x43EC);
  for (int i = 0; i < 3; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  addComment(ofsMisc, "error 1");
  ifs.seek(0x47B2);
  for (int i = 0; i < 1; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  addComment(ofsMisc, "error 2");
  ifs.seek(0x47DD);
  for (int i = 0; i < 1; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  addComment(ofsMisc, "error 3");
  ifs.seek(0x480B);
  for (int i = 0; i < 1; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  addComment(ofsMisc, "error 4");
  ifs.seek(0x488D);
  for (int i = 0; i < 1; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  addComment(ofsMisc, "error 5");
  ifs.seek(0x48B8);
  for (int i = 0; i < 1; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  addComment(ofsMisc, "error 6");
  ifs.seek(0x4A10);
  for (int i = 0; i < 1; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  addComment(ofsMisc, "error 7");
  ifs.seek(0x4A40);
  for (int i = 0; i < 1; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  addComment(ofsMisc, "error 8");
  ifs.seek(0x4AF3);
  for (int i = 0; i < 1; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  addComment(ofsMisc, "error 9");
  ifs.seek(0x4B15);
  for (int i = 0; i < 1; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  // TODO: more error messages at 0x718D+.
  // probably don't matter, though.
  
  addComment(ofsMisc, "yes/no prompt");
  ifs.seek(0x4C9D);
  for (int i = 0; i < 1; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  addComment(ofsMisc, "character select 1a");
  ifs.seek(0x51A8);
  for (int i = 0; i < 2; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  addComment(ofsMisc, "character select 1b");
  ifs.seek(0x51BD);
  for (int i = 0; i < 1; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  addComment(ofsMisc, "character select 2a");
  ifs.seek(0x51D1);
  for (int i = 0; i < 2; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  addComment(ofsMisc, "character select 2b");
  ifs.seek(0x51E6);
  for (int i = 0; i < 1; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  addComment(ofsMisc, "character select 3a");
  ifs.seek(0x51FA);
  for (int i = 0; i < 2; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  addComment(ofsMisc, "character select 3b");
  ifs.seek(0x520F);
  for (int i = 0; i < 1; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  addComment(ofsMisc, "character select prompt");
  ifs.seek(0x521D);
  for (int i = 0; i < 1; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  addComment(ofsMisc, "sound test 1");
  ifs.seek(0x61FD);
  for (int i = 0; i < 1; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  addComment(ofsMisc, "title screen options");
  ifs.seek(0x648D);
  for (int i = 0; i < 1; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  addComment(ofsMisc, "title screen copyright");
  ifs.seek(0x6498);
  for (int i = 0; i < 1; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }
  
  addComment(ofsMisc, "intro");
  ifs.seek(0x6539);
  for (int i = 0; i < 3; i++) {
    dumpString(ifs, ofsMisc, table, ifs.tell());
  }

  //==============================
  // location names
  //==============================

  std::ofstream ofsPlaces("script/script_places.txt");
  
  // TODO: location name "mori" at 0x4479 must be manually moved,
  // because it won't be long enough for a redirect sequence
  addComment(ofsPlaces, "location names");
  for (int i = 0; i < 20; i++) {
    ifs.seek(0x440B + (2 * i));
    int ptr = ifs.readu16le();
    ifs.seek(ptr);
    int len = ifs.readu8();
    dumpString(ifs, ofsPlaces, table, ifs.tell(), len);
  }

  //==============================
  // misc fixed-length strings
  //==============================

  std::ofstream ofsFixedLen("script/script_fixedlen.txt");
  
  addComment(ofsFixedLen, "battle: enemy names");
  for (int i = 0; i < 12; i++) {
    ifs.seek(0x2AE0 + (0x24 * i));
    for (int j = 0; j < 3; j++) {
      dumpString(ifs, ofsFixedLen, table, ifs.tell(), 8);
    }
  }
  
  addComment(ofsFixedLen, "player character full names");
  for (int i = 0; i < 3; i++) {
    ifs.seek(0x3C20 + (2 * i));
    int ptr = ifs.readu16le();
    ifs.seek(ptr);
    dumpString(ifs, ofsFixedLen, table, ifs.tell(), 8);
  }
  
  addComment(ofsFixedLen, "sound test 2");
  ifs.seek(0x6208);
  for (int i = 0; i < 43; i++) {
    dumpString(ifs, ofsFixedLen, table, ifs.tell(), 15);
  }

  //==============================
  // credits
  //==============================

  std::ofstream ofsCred("script/script_credits.txt");
  
  addComment(ofsCred, "credits");
  {
    int searchPos = 0x6A75;
    while (!ifs.eof() && (searchPos < 0x6D00)) {
      ifs.seek(searchPos);
      
      int next = ifs.readu8();
      
      if (next == 0x02) {
        dumpString(ifs, ofsCred, table, searchPos + 1, 9);
        searchPos = ifs.tell();
      }
      else if (next == 0x03) {
        dumpString(ifs, ofsCred, table, searchPos + 1, 16);
        searchPos = ifs.tell();
      }
      else if (next == 0x08) {
        dumpString(ifs, ofsCred, table, searchPos + 1, 24);
        searchPos = ifs.tell();
      }
      else {
        ++searchPos;
      }
    }
  }
  
//  for (NametagToTranslationMap::iterator it = nametagMap.begin();
//       it != nametagMap.end();
//       ++it) {
//    cout << "\"" << it->first << "\",\"" << it->second << "\"" << std::endl; 
//  }
  
  return 0; 
}
