#include <vector>
#include <ircodes.h>
#include <codegen.h>
#include <symtab.h>
using namespace std;

string loadArrAddr(ofstream & f, const sym* symb,
                        vector<sym*> offsetSymb, 
                        vector<string> offset, int type, string pos, vector<pfxOpr> PfxOprs) {
  /*
    * pos:  0 for dst (use $a0, $a3)
    *       1 for src1 (use $a1, $a3)
    *       2 for src2 (use $a2, $a3)
  */

  if(!symb || type == 0 ) {
    // cout << "string loadArrAddr: Called for non-array" << endl;
    return "";
  }

  string addrReg = "$a" + pos;          // store address here if non-constant offset or "->"
	string symReg = reg2str[symb->reg];   // base address of the base symbol
	bool allConst = true;
	int constOffset = 0;
  f << "\t #### <Load address> ###" << endl;
  
  if (type == 1) {
    vector<int> dimSize;
    Arr *a = (Arr *) symb->type;
    for(auto dim: a->dims) {
      int * sizePtr = eval(dim);
      if(!sizePtr) {
        cout << "string loadArrAddr: eval(dim) failed for a dimension" << endl;
        dimSize.push_back(INT16_MAX);
      }
      else dimSize.push_back(*sizePtr);
    }
    
    /* for array part */
    if(dimSize.size() > offset.size())
      cout << "string loadArrAddr: some problem in semantic analysis" << endl;
    
    int dimWidth = getSize(a->item);
    for(int i = dimSize.size()-1; i>=0; i--) {
      if(!offsetSymb[i]) {
        /* constant  operand*/
        constOffset += stoi(offset[i]) * dimWidth;
      }
      else {
        /* variable offset */
        /* eg. lw $a3, -44($fp) */
        f << '\t' << "lw $a3, -" + to_string(offsetSymb[i]->offset) + "($fp)"
          << " # load " + offsetSymb[i]->name << endl;
        /* eg. mul $a3, $a3, dimWidth */
        if (dimWidth != 1)
          f << '\t' << "mul $a3, $a3, " + to_string(dimWidth)
            << " # multiply with dimWidth" << endl;
        
        if(allConst){
          allConst = false;
          // base + currOffset
          /* eg. addu $a0, $t0, $a3 */
          f << '\t' << "addu " + addrReg + ", " + symReg +", $a3"
            << " # offset calc" << endl;
        }
        else 
          // offsetTillNow + currOffset
          /* eg. addu $a0, $a0, $a3 */
          f << '\t' << "addu " + addrReg + ", " + addrReg + ", $a3" 
            << " # offset calc" << endl;
      }
      dimWidth *= dimSize[i];
    }
    /* eg. "10($t0)" */
    if(allConst) return to_string(constOffset) + "("+symReg+")";
    /* eg. "$a0" */
    else {
      if (constOffset)
      /* addu $a0, $a0, constOffset */
      f << '\t' << "addu " + addrReg + ", " + addrReg + ", " + to_string(constOffset) 
            << " # add const offset" << endl;
      return "("+addrReg+")";
    }
  }
  
  else if(type == 2) { // struct: ".", "->"
		f << '\t' << "move "<< addrReg + ", " << symReg << endl;
		for(auto pfx: PfxOprs) {
			// cout << pfx.name << endl;
			if(pfx.type == ".") {
				f << '\t' << "addu " + addrReg + ", " + addrReg + ", " << to_string(pfx.symb->offset) 
            << " # offset calc" << endl;
			}
			else if(pfx.type == ">") {
				f << '\t' << "lw " << addrReg << ", " << "("+addrReg+")" << endl;
				f << '\t' << "addu " + addrReg + ", " + addrReg + ", " << to_string(pfx.symb->offset) 
            << " # offset calc" << endl;
			}
		}
		return "("+addrReg+")";
  }

  /* for pointer part */
  // TODO: pointer
  return "aa";
}



void parseStruct(string & q, int &type, vector<pfxOpr> &PfxOprs, Type * &finalType) {
	vector <string> tokens = {".", ">"};
	int nxtPos = Find_first_of(q, tokens);
	if(nxtPos >= 0) {
		type = 2;
		cout <<"Struct parse " << q << endl;
		string tmp = q.substr(0, nxtPos);
		if(q[nxtPos] == '>')tmp.pop_back();
		cout << tmp << endl;
		/* search in sym table */
		sym* st_sym = SymRoot->gLookup(tmp);
		Type* st_type;
		if(st_sym)
			st_type = st_sym->type;
		else {
			cout << "can't find " + tmp + " in symtab" << endl;
			return;
		}

		/* push first symbol */
		pfxOpr p;
		p.name = tmp;
		p.symb = st_sym;
		p.type = "---";
		PfxOprs.push_back(p);

		int nnxtPos = Find_first_of(q, tokens, nxtPos+1);
		while(nnxtPos >= 0){
			pfxOpr p;
			p.type = q[nxtPos]; // ".", ">"
			p.name = q.substr(nxtPos+1, nnxtPos-nxtPos-1);
			if(q[nnxtPos] == '>') p.name.pop_back();
			
			if(p.type == ">") { // pointer to struct
				if(!isPtr(st_type)) cout << "-> for non-pointer" << endl;
				st_type = ((Ptr *) st_type)->pt;
			}
			if(!isStruct(st_type)) cout << p.name + " is not an struct" << endl;
			st_sym = findStructChild(st_type, p.name);
			st_type = st_sym->type;
			p.symb = st_sym;
			PfxOprs.push_back(p);
			cout << p.type + "  " + p.name << endl; //!

			nxtPos = nnxtPos;
			nnxtPos = Find_first_of(q, tokens, nxtPos + 1);
		}
		if(nxtPos != q.size()) {
			pfxOpr p;
			p.type = q[nxtPos]; // ".", ">"
			p.name = q.substr(nxtPos+1);
			if(q[nnxtPos] == '>') p.name.pop_back();

			if(p.type == ">") { // pointer to struct
				if(!isPtr(st_type)) cout << "-> for non-pointer" << endl;
				st_type = ((Ptr *) st_type)->pt;
			}
			if(!isStruct(st_type)) cout << p.name + " is not an struct" << endl;
			st_sym = findStructChild(st_type, p.name);
			st_type = st_sym->type;
			p.symb = st_sym;
			PfxOprs.push_back(p);
			cout << p.type + "  " + p.name << endl; //!
		}
		q = tmp;
		finalType = st_type;
	}
}


void memCopy(std::ofstream &f, reg_t src, reg_t dst, int size) {
  string srcReg = reg2str[src], dstReg = reg2str[dst], freeReg = "$a3";
  string loadInstr[] = {"lw", "ls", "lb"};
  string storeInstr[] = {"sw", "ss", "lb"};
  int chnkSize = 4, instr = 0, remSize = size;
  while(remSize > 0) {
    if(remSize < chnkSize) {
      chnkSize /= 2;
      instr++;
      continue;
    }
    /* eg. lw $a3, 20($a0) */
    f << '\t' << loadInstr[instr] + " " + freeReg + ", "
      << to_string(size-remSize) + "("  << srcReg + ")";
    /* eg. sw $a3, 20($a1) */
    f << '\t' << storeInstr[instr] + " " + freeReg + ", "
      << to_string(size-remSize) + "("  << srcReg + ")";
    remSize -= chnkSize;
  }
}


sym* findStructChild(Type* st_type, string chName) {
	Base * b = (Base *) st_type;
	for (auto ch: b->subDef->syms) {
		if(ch->name == chName) return ch;
	}
	return NULL;
}


size_t Find_first_of(string s, vector<string> tokens, size_t pos) {
	size_t nxtPos = -1;
	for(auto tok: tokens) {
		nxtPos = min(nxtPos, s.find_first_of(tok, pos));
	}
	return nxtPos;
}