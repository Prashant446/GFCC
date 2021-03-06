// AUM SHREEGANESHAAYA NAMAH||
#ifndef __GFCC_ASM__
#define __GFCC_ASM__

enum reg_t {
  zero,                                   // constant 0
  at,                                     // reserved for the assembler
  v0, v1,                                 // result registers
  a0, a1, a2, a3,                         // argument registers 1···4
  t0, t1, t2, t3, t4, t5, t6, t7,         // temporary registers 0···7
  s0, s1, s2, s3, s4, s5, s6, s7,         // saved registers 0···7
  t8, t9,                                 // temporary registers 8···9
  k0, k1,                                 // kernel registers 0···1
  gp, sp, fp, ra,                          // global data ptr, stack ptr, frame ptr, return addr

  f0, f1, f2, f3,                         // Function-returned values
  f4, f5, f6, f7, f8, f9, f10, f11,       // Temporary values
  f16, f17, f18, f19,                     // More Temporary values
  f12, f13, f14, f15,                     // Arguments passed into a function
  f20, f21, f31,                          // Saved values
};

#include <fstream>
#include <vector>
#include <stack>
#include <map>
#include <unordered_map>

#include <gfcc_lexer.h>
#include <ircodes.h>
#include <symtab.h>

extern std::string reg2str[];

extern std::string freg2str[];

extern class sym* regDscr[];

extern bool semanticErr;

struct pfxOpr {
  /* contains info about 1 postfix opr: "[", "->", "." */
  /* sym* if identifier in [], from subdef for struct */
  sym* symb = NULL;
  /* symbol name */
  std::string name; // for array
  /* "[]", ".", ">" */
  std::string type;
  /* if [] is for pointer */
  bool isPtr = false;
};

struct deltaOpd {
  /* contains one delta + other useful info about a symbol */
  sym* Sym = NULL;
  int NxtUse = -1;
  bool Alive = true;
  // no. of * prepended in the symbol
  int derefCnt = 0;
  std::vector<pfxOpr> PfxOprs;
  /* 0: simple, 1: 2: struct/array */
  int Type = 0;
  /* filled in case of struct, array, etc */
  class Type* FinalType = NULL;
};

struct deltaNxtUse {
  /* contains one delta + other useful info about dst, src1, src2 */
  deltaOpd dst;
  deltaOpd src1;
  deltaOpd src2;
};

class _nxtUse {
  /* Contains all the deltas of main block */
  public:
    // last changes popped - has sym*s of current line
    deltaNxtUse lastdelta;
    // changes in nxtuse information
    std::vector<deltaNxtUse> deltas;
    void clear();
    deltaNxtUse step();
};

struct oprRegs {
  reg_t dstReg;
  reg_t src1Reg;
  reg_t src2Reg;
};

struct inst_t {
  std::string load_instr;
  std::string store_instr;
  std::string load_const;
  std::string move_instr;
};

struct tmp_regs {
  reg_t retreg1;
  reg_t retreg2;
  reg_t exreg;
};

void regFlush(std::ofstream &, reg_t, bool);

void regMap(std::ofstream &, reg_t, sym*, bool);

void resetRegMaps(std::ofstream &, bool);

void dumpASM(std::ofstream &, std::vector<irquad_t> &); // convert IR code to ASM (mainly MIPS, for "spim" simulator)

reg_t getSymReg(const std::string &);

inst_t getInst(class Type *);

oprRegs getReg(std::ofstream &, const irquad_t &q);

tmp_regs getTmpRegs(class Type *);

void genASM(std::ofstream &, irquad_t &);

void funcStart(std::ofstream &, const irquad_t &); // preliminaries at beginning of function

void funcEnd(std::ofstream &, const irquad_t &); // preliminaries at end of function

void binOpr(std::ofstream &, irquad_t &); // handle binary operators

void assn(std::ofstream &, const irquad_t &);

int getNxtLeader(std::vector<irquad_t> &, int);

std::string getSymName(std::string); // get symbol name for struct, array, etc

void resetCodegen();

void parseStruct(std::string &q, deltaOpd & Opd);

std::size_t Find_first_of(std::string s, std::vector<std::string> tokens, std::size_t pos = 0U);

sym* findStructChild(Type* st_type, std::string chName);

Type* ptrChildType(Type * t);

std::vector<int> getDimSize(Arr* a);

int getWidth(std::vector<int> dimSize);

std::string loadArrAddr(std::ofstream &, deltaOpd & Opd, std::string pos);

void memCopy(std::ofstream &, reg_t src, reg_t dst, int size);

#endif
