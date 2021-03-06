#include <vector>
#include <ircodes.h>
#include <codegen.h>
#include <symtab.h>
using namespace std;

string loadArrAddr(ofstream & f, deltaOpd & Opd, string pos) {
  /*
    * pos:  0 for dst (use $a0, $a3)
    *       1 for src1 (use $a1, $a3)
    *       2 for src2 (use $a2, $a3)
  */
  if(!Opd.Sym || Opd.Type == 0 ) {
    // cout << "string loadArrAddr: Called for non-array" << endl;
    return "";
  }
  // flush all registers in stack

  string addrReg = "$a" + pos;          // store address here if non-constant offset or "->"
	string symReg = reg2str[Opd.Sym->reg];   // base address of the base symbol
	bool allConst = true;
	int constOffset = 0;
  f << "\t #### <Load address> ###" << endl;
  bool first = true;

  if(Opd.Type == 2) { // struct: ".", "->"
		f << '\t' << "move "<< addrReg + ", " << symReg << endl;
		for(auto pfx: Opd.PfxOprs) {
			// cout << pfx.name << endl;
			if(pfx.type == ".") {
        if (pfx.symb->offset != 0)
         f << '\t' << "addu " + addrReg + ", " + addrReg + ", " << to_string(pfx.symb->offset) 
            << " # offset calc" << endl;
			}
			else if(pfx.type == ">") {
        if(!first) // first pointer is by default loaded
				  f << '\t' << "lw " << addrReg << ", " << "("+addrReg+")" << endl;
        if (pfx.symb->offset != 0)
				  f << '\t' << "addu " + addrReg + ", " + addrReg + ", " << to_string(pfx.symb->offset) 
            << " # offset calc" << endl;
			}
      else if (pfx.type == "[]") { // "[]"
        if(pfx.symb == NULL) {
          if (pfx.name != "0")
          f << '\t' << "addu " + addrReg + ", " + addrReg + ", " << pfx.name
            << " # offset calc" << endl;
          if(!first && pfx.isPtr)
  				  f << '\t' << "lw " << addrReg << ", " << "("+addrReg+")" << endl;
        }
        else {
          if(pfx.symb->reg != zero)
            f << '\t' << "move $a3, " + reg2str[pfx.symb->reg]
              << " # load " + pfx.symb->name << endl;
          else 
            f << '\t' << "lw $a3, -" + to_string(pfx.symb->offset) + "($fp)"
              << " # load " + pfx.symb->name << endl;
          
          f << '\t' << "mul $a3, $a3, " + pfx.name
            << " # width calc" << endl;
          f << '\t' << "addu " + addrReg + ", " + addrReg + ", $a3"
            << " # offset calc" << endl;
          if(!first && pfx.isPtr)
  				  f << '\t' << "lw " << addrReg << ", " << "("+addrReg+")" << endl;
        }
      }
      if(first) first = false;
		}

    if(Opd.derefCnt > 0) {
      for(int i = 0; i< Opd.derefCnt -1 ; i++)
        f << '\t' << "lw " << addrReg << ", " << "("+addrReg+")" 
          << " # dereference pointer" << endl;
    }
    
  }  // Opd.Type == 2
  
	return "("+addrReg+")";
  /* for pointer part */
  // TODO: pointer
  return "aa";
}



void parseStruct(string & q, deltaOpd & Opd) {
  sym* st_sym = NULL;
  Type * st_type = NULL;
  // count '*'
  for(int i = 0; i < q.size(); i++) {
    if(q[i] == '*') {
      Opd.derefCnt++;
    }
  }
  if(Opd.derefCnt > 0) {
    Opd.Type = 2;
    q = q.substr(Opd.derefCnt);
    st_sym = SymRoot->gLookup(q);
    if(st_sym)
			st_type = st_sym->type;
    else {
      // cout << "can't find " + q + " in symtab" << endl;
    }
  }

	vector <string> tokens = {".", ">", "["};
	int currPos = Find_first_of(q, tokens);
	if(currPos >= 0) {
		Opd.Type = 2;
		// cout <<"Struct parse " << q << endl;
		string tmp = q.substr(0, currPos);
		if(q[currPos] == '>')tmp.pop_back();
		// cout << "Base: " + tmp << endl;
		/* search in sym table */
		st_sym = SymRoot->gLookup(tmp);
		if(st_sym)
			st_type = st_sym->type;
		else {
			// cout << "can't find " + tmp + " in symtab" << endl;
			return;
		}

		/* push first symbol */
		pfxOpr p;
		p.name = tmp;
		p.symb = st_sym;
		p.type = "---";
		// Opd.PfxOprs.push_back(p);

    vector<int> dimSize;
    int counter = 0, dimCount = 0, dimWidth = 1;
    int nxtPos = Find_first_of(q, tokens, currPos+1);
		while(currPos >= 0){
			pfxOpr p;
      p.type = q[currPos]; // ".", ">", "["

      if(p.type == ">" || p.type == ".") { // ".", "->"
        p.name = q.substr(currPos+1, nxtPos-currPos-1);
        if(q[nxtPos] == '>') p.name.pop_back();
        
        if(p.type == ">") { // pointer to struct
          if(!isPtr(st_type)) cout << "-> for non-pointer" << endl;
          st_type = ((Ptr *) st_type)->pt;
        }
        if(!isStruct(st_type)) cout << p.name + " is not an struct" << endl;
			  st_sym = findStructChild(st_type, p.name);
			  st_type = st_sym->type;
        p.symb = st_sym;
      }

      else if(p.type == "[") { // "[]" -- either array or pointer

        p.type = "[]";
        int brkClosePos = q.find_first_of("]", currPos+1);
        if(brkClosePos == -1) cout << "parseStruct:: Error: can't find ']'" << endl;
        p.name = q.substr(currPos+1, brkClosePos-currPos-1);
        p.symb = SymRoot->gLookup(p.name); // NULL if constant
        nxtPos = Find_first_of(q, tokens, brkClosePos+1);

        if(dimCount == 0) { // start of array or pointer
          if(!isArr(st_type) && !isPtr(st_type)) cout << "-> for invalid type" << endl;
          
          if(isArr(st_type)) {
            Arr* a = (Arr *) st_type;
            st_type =  a->item;
            dimSize = getDimSize(a);
            dimWidth = getWidth(dimSize);
            dimCount = dimSize.size();
            counter = 1;
          }
          
          else {
            // element width == dimWidth
            dimWidth = 1;
            counter = 0;
            st_type = ptrChildType(st_type);
            p.isPtr = true;
          }
        }
        
        int dimOffs = getSize(st_type) * dimWidth; // dimWidth * elSize
        if(!p.symb) { // constant offset
          p.name = to_string(stoi(p.name) * dimOffs);
        }
        else p.name = to_string(dimOffs);

        if(counter == dimCount){ // end multidim array;
          counter = 0;
          dimCount = 0;
        }
        else { // carry on multi-dim array
          // get width of nxt dimension
          dimWidth /= dimSize[counter];
          counter++;
        }
      }

			Opd.PfxOprs.push_back(p);
			// cout << p.type + "  " + p.name + " " << counter << endl; //!
			currPos = nxtPos;
			nxtPos = Find_first_of(q, tokens, currPos + 1);
		}
		q = tmp;
		Opd.FinalType = st_type;
	} // currPos >= 0
  if(Opd.derefCnt > 0) {
    if(!st_type) cout << "Can't deref Pointer" << endl;
    for(int i = 0; i<Opd.derefCnt; i++) 
      st_type = ptrChildType(st_type);
		Opd.FinalType = st_type;
  }
}


void memCopy(std::ofstream &f, reg_t src, reg_t dst, int size) {
  if(size < 1) return;

  string srcReg = reg2str[src], dstReg = reg2str[dst], freeReg = "$a3";
  string loadInstr[] = {"lw", "ls", "lb"};
  string storeInstr[] = {"sw", "ss", "lb"};
  
  f << '\t' << "### <memcopy> size = " + to_string(size)
    << " from " + reg2str[src] + " to " + reg2str[dst] + " ###"<< endl;
  int chnkSize = 4, instr = 0, remSize = size;
  while(remSize > 0) {
    if(remSize < chnkSize) {
      chnkSize /= 2;
      instr++;
      continue;
    }

    /* eg. lw $a3, 20($a0) */
    f << '\t' << loadInstr[instr] + " " + freeReg + ", "
      << to_string(size-remSize) + "("  << srcReg + ")" << endl;
    /* eg. sw $a3, 20($a1) */
    f << '\t' << storeInstr[instr] + " " + freeReg + ", "
      << to_string(size-remSize) + "("  << dstReg + ")" << endl;
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

Type* ptrChildType(Type * t) {
  if(!isPtr(t)) return NULL;
  Ptr* p = (Ptr *) clone(t);
  if(p->ptrs.size()==1) return p->pt;
  else {
    p->ptrs.pop_back();
    return p;
  }
}

vector<int> getDimSize(Arr* a) {
  vector<int> dimSize;
  for(auto dim: a->dims) {
    int * sizePtr = eval(dim);
    if(!sizePtr) {
      dimSize.push_back(INT16_MAX);
    }
    else dimSize.push_back(*sizePtr);
  }
  return dimSize;
}

int getWidth(vector<int> dimSize) {
  int width = 1;
  for(int i = 1; i < dimSize.size(); i++) {
    width *= dimSize[i];
  }
  // cout << "width " << width << endl;
  return width;
}

size_t Find_first_of(string s, vector<string> tokens, size_t pos) {
	size_t nxtPos = -1;
	for(auto tok: tokens) {
		nxtPos = min(nxtPos, s.find_first_of(tok, pos));
	}
	return nxtPos;
}
