#define FILE void
#include "defines.h"
struct optcode opt_table[] = {
  { "ADC A,(X)", 0x86, 0 },
  { "ADC A,#x", 0x88, 1 },
  { "ADC A,!?+X", 0x95, 2 },
  { "ADC A,!?+Y", 0x96, 2 },
  { "ADC A,!?", 0x85, 2 },
  { "ADC A,[x+X]", 0x87, 1 },
  { "ADC A,[x]+Y", 0x97, 1 },
  { "ADC A,x+X", 0x94, 1 },
  { "ADC A,x", 0x84, 1 },
  { "ADC (X),(Y)", 0x99, 0 },
  { "ADC x,#x", 0x98, 0xB },
  { "ADC x,x", 0x89, 0xB },
  { "ADDW YA,x", 0x7A, 1 },
  { "AND1 C,/?.~", 0x6A, 0xF },
  { "AND1 C,?.~", 0x4A, 0xF },
  { "AND A,(X)", 0x26, 0 },
  { "AND A,#x", 0x28, 1 },
  { "AND A,!?+X", 0x35, 2 },
  { "AND A,!?+Y", 0x36, 2 },
  { "AND A,!?", 0x25, 2 },
  { "AND A,[x+X]", 0x27, 1 },
  { "AND A,[x]+Y", 0x37, 1 },
  { "AND A,x+X", 0x34, 1 },
  { "AND A,x", 0x24, 1 },
  { "AND (X),(Y)", 0x39, 0 },
  { "AND x,#x", 0x38, 0xB },
  { "AND x,x", 0x29, 0xB },
  { "ASL !?", 0x0C, 2 },
  { "ASL A", 0x1C, 0 },
  { "ASL x+X", 0x1B, 1 },
  { "ASL x", 0x0B, 1 },

  { "BRK", 0x0F, 0 },
  { "BBC x.~,x", 0x03, 0xD },
  { "BBS x.~,x", 0x03, 0xC },
  { "BCC x", 0x90, 0xE },
  { "BCS x", 0xB0, 0xE },
  { "BEQ x", 0xF0, 0xE },
  { "BMI x", 0x30, 0xE },
  { "BNE x", 0xD0, 0xE },
  { "BPL x", 0x10, 0xE },
  { "BRA x", 0x2F, 0xE },
  { "BVC x", 0x50, 0xE },
  { "BVS x", 0x70, 0xE },

  { "CALL !?", 0x3F, 2 },
  { "CBNE x+X,x", 0xDE, 0xA },
  { "CBNE x,x", 0x2E, 0xA },
  { "CLR1 x.~", 0x02, 4 },
  { "CLR1 x,~", 0x02, 4 },
  { "CLRC", 0x60, 0 },
  { "CLRP", 0x20, 0 },
  { "CLRV", 0xE0, 0 },
  { "CMP (X),(Y)", 0x79, 0 },
  { "CMP A,(X)", 0x66, 0 },
  { "CMP A,#x", 0x68, 1 },
  { "CMP A,!?+X", 0x75, 2 },
  { "CMP A,!?+Y", 0x76, 2 },
  { "CMP A,!?", 0x65, 2 },
  { "CMP A,[x+X]", 0x67, 1 },
  { "CMP A,[x]+Y", 0x77, 1 },
  { "CMP A,x+X", 0x74, 1 },
  { "CMP A,x", 0x64, 1 },
  { "CMP X,!?", 0x1E, 2 },
  { "CMP X,#x", 0xC8, 1 },
  { "CMP X,x", 0x3E, 1 },
  { "CMP Y,!?", 0x5E, 2 },
  { "CMP Y,#x", 0xAD, 1 },
  { "CMP Y,x", 0x7E, 1 },
  { "CMP x,#x", 0x78, 0xB },
  { "CMP x,x", 0x69, 0xB },
  { "CMPW YA,x", 0x5A, 1 },

  { "DAA", 0xDF, 0 },
  { "DAA A", 0xDF, 0 },
  { "DAS", 0xBE, 0 },
  { "DAS A", 0xBE, 0 },
  { "DBNZ Y,x", 0xFE, 0xE },
  { "DBNZ x,x", 0x6E, 0xA },
  { "DEC A", 0x9C, 0 },
  { "DEC !?", 0x8C, 2 },
  { "DEC X", 0x1D, 0 },
  { "DEC Y", 0xDC, 0 },
  { "DEC x+X", 0x9B, 1 },
  { "DEC x", 0x8B, 1 },
  { "DECW x", 0x1A, 1 },
  { "DI", 0xC0, 0 },
  { "DIV YA,X", 0x9E, 0 },

  { "EI", 0xA0, 0 },
  { "EOR1 C,?.~", 0x8A, 0xF },
  { "EOR A,(X)", 0x46, 0 },
  { "EOR A,#x", 0x48, 1 },
  { "EOR A,!?+X", 0x55, 2 },
  { "EOR A,!?+Y", 0x56, 2 },
  { "EOR A,!?", 0x45, 2 },
  { "EOR A,[x+X]", 0x47, 1 },
  { "EOR A,[x]+Y", 0x57, 1 },
  { "EOR A,x+X", 0x54, 1 },
  { "EOR A,x", 0x44, 1 },
  { "EOR (X),(Y)", 0x59, 0 },
  { "EOR x,#x", 0x58, 0xB },
  { "EOR x,x", 0x49, 0xB },

  { "INC !?", 0xAC, 2 },
  { "INC A", 0xBC, 0 },
  { "INC X", 0x3D, 0 },
  { "INC Y", 0xFC, 0 },
  { "INC x+X", 0xBB, 1 },
  { "INC x", 0xAB, 1 },
  { "INCW x", 0x3A, 1 },

  { "JMP !?", 0x5F, 2 },
  { "JMP [!?+X]", 0x1F, 2 },

  { "LSR !?", 0x4C, 2 },
  { "LSR A", 0x5C, 0 },
  { "LSR x+X", 0x5B, 1 },
  { "LSR x", 0x4B, 1 },

  { "MOV A,#x", 0xE8, 1 },
  { "MOV A,(X)+", 0xBF, 0 },
  { "MOV A,(X)", 0xE6, 0 },
  { "MOV A,!?+X", 0xF5, 2 },
  { "MOV A,!?+Y", 0xF6, 2 },
  { "MOV A,!?", 0xE5, 2 },
  { "MOV A,X", 0x7D, 0 },
  { "MOV A,Y", 0xDD, 0 },
  { "MOV A,[x+X]", 0xE7, 1 },
  { "MOV A,[x]+Y", 0xF7, 1 },
  { "MOV A,x+X", 0xF4, 1 },
  { "MOV A,x", 0xE4, 1 },
  { "MOV SP,X", 0xBD, 0 },
  { "MOV X,A", 0x5D, 0 },
  { "MOV X,!?", 0xE9, 2 },
  { "MOV X,SP", 0x9D, 0 },
  { "MOV X,#x", 0xCD, 1 },
  { "MOV X,x+Y", 0xF9, 1 },
  { "MOV X,x", 0xF8, 1 },
  { "MOV (X)+,A", 0xAF, 0 },
  { "MOV (X),A", 0xC6, 0 },
  { "MOV [x+X],A", 0xC7, 1 },
  { "MOV [x]+Y,A", 0xD7, 1 },
  { "MOV !?+X,A", 0xD5, 2 },
  { "MOV !?,A", 0xC5, 2 },
  { "MOV !?,X", 0xC9, 2 },
  { "MOV !?+Y,A", 0xD6, 2 },
  { "MOV !?,Y", 0xCC, 2 },
  { "MOV Y,!?", 0xEC, 2 },
  { "MOV Y,A", 0xFD, 0 },
  { "MOV Y,#x", 0x8D, 1 },
  { "MOV Y,x+X", 0xFB, 1 },
  { "MOV Y,x", 0xEB, 1 },
  { "MOV x,A", 0xC4, 1 },
  { "MOV x,#x", 0x8F, 0xB },
  { "MOV x,X", 0xD8, 1 },
  { "MOV x+X,A", 0xD4, 1 },
  { "MOV x+X,Y", 0xDB, 1 },
  { "MOV x+Y,X", 0xD9, 1 },
  { "MOV x,Y", 0xCB, 1 },
  { "MOV x,x", 0xFA, 0xB },
  { "MOVW YA,x", 0xBA, 1 },
  { "MOVW x,YA", 0xDA, 1 },
  { "MOV1 C,?.~", 0xAA, 0xF },
  { "MOV1 ?.~,C", 0xCA, 0xF },
  { "MUL YA", 0xCF, 0 },

  { "NOP", 0x00, 0 },
  { "NOT1 ?.~", 0xEA, 0xF },
  { "NOTC", 0xED, 0 },

  { "OR1 C,/?.~", 0x2A, 0xF },
  { "OR1 C,?.~", 0x0A, 0xF },
  { "OR A,(X)", 0x06, 0 },
  { "OR A,#x", 0x08, 1 },
  { "OR A,!?+X", 0x15, 2 },
  { "OR A,!?+Y", 0x16, 2 },
  { "OR A,!?", 0x05, 2 },
  { "OR A,[x+X]", 0x07, 1 },
  { "OR A,[x]+Y", 0x17, 1 },
  { "OR A,x+X", 0x14, 1 },
  { "OR A,x", 0x04, 1 },
  { "OR (X),(Y)", 0x19, 0 },
  { "OR x,#x", 0x18, 0xB },
  { "OR x,x", 0x09, 0xB },

  { "PCALL x", 0x4F, 1 },
  { "POP A", 0xAE, 0 },
  { "POP PSW", 0x8E, 0 },
  { "POP X", 0xCE, 0 },
  { "POP Y", 0xEE, 0 },
  { "PUSH A", 0x2D, 0 },
  { "PUSH PSW", 0x0D, 0 },
  { "PUSH X", 0x4D, 0 },
  { "PUSH Y", 0x6D, 0 },

  { "RET", 0x6F, 0 },
  { "RETI", 0x7F, 0 },
  { "ROL A", 0x3C, 0 },
  { "ROL !?", 0x2C, 2 },
  { "ROL x+X", 0x3B, 1 },
  { "ROL x", 0x2B, 1 },
  { "ROR A", 0x7C, 0 },
  { "ROR !?", 0x6C, 2 },
  { "ROR x+X", 0x7B, 1 },
  { "ROR x", 0x6B, 1 },

  { "SBC A,(X)", 0xA6, 0 },
  { "SBC A,#x", 0xA8, 1 },
  { "SBC A,!?+X", 0xB5, 2 },
  { "SBC A,!?+Y", 0xB6, 2 },
  { "SBC A,!?", 0xA5, 2 },
  { "SBC A,[x+X]", 0xA7, 1 },
  { "SBC A,[x]+Y", 0xB7, 1 },
  { "SBC A,x+X", 0xB4, 1 },
  { "SBC A,x", 0xA4, 1 },
  { "SBC (X),(Y)", 0xB9, 0 },
  { "SBC x,#x", 0xB8, 0xB },
  { "SBC x,x", 0xA9, 0xB },
  { "SET1 x.~", 0x02, 3 },
  { "SET1 x,~", 0x02, 3 },
  { "SETC", 0x80, 0 },
  { "SETP", 0x40, 0 },
  { "SLEEP", 0xEF, 0 },
  { "STOP", 0xFF, 0 },
  { "SUBW YA,x", 0x9A, 1 },

  { "TCALL ~", 0x01, 5 },
  { "TCLR1 !?", 0x4E, 2 },
  { "TSET1 !?", 0x0E, 2 },

  { "XCN A", 0x9F, 0 },

  { "E", 0x100, 0xFF }
};