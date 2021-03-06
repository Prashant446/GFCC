// AUM SHREEGANESHAAYA NAMAH||

/************************ NOTE TO THE DEVELOPER ************************
 * SEE <proj_root>/src/include/types2.h for description of various classes,
   members, methods, etc.
 * Search "TODO" for things to do.
 * Search "ASSUMPTIONS" for assumptions.
 * Execution using: g++ -std=c++11 -Iinclude [-DTEST_TYPES_2] [-DDEBUG_TYPES_2] types2.cpp && ./a.out
 * String related stuff after class methods, before test suites.
************************************************************************/

#include <iostream>
#include <iomanip>
#include <unordered_map>

#include <parser.tab.h>
#include <gfcc_colors.h>
#include <types2.h>
#include <typo.h>
#include <symtab.h>
#include <ircodes.h>

// debugging does not work as of yet - actually not required
#ifdef DEBUG_TYPES_2
const static bool dbg = true;
#else
const static bool dbg = false;
#endif

using namespace std;

unordered_map<base_t, int> priority1 =
    {{CHAR_B, 0}, {SHORT_B, 1}, {INT_B, 2}, {LONG_B, 3}, {LONG_LONG_B, 4}, {FLOAT_B, 5}, {DOUBLE_B, 6}, {LONG_DOUBLE_B, 7}};


bool brackPut = false;

bool tpdef = false;

void resetTypes() { // reset appropriate global variables
    brackPut = tpdef = false;
}


/******************************************************/
/****************** struct "_qual_t" ******************/
/******************************************************/

_qual_t::_qual_t () { }

_qual_t::_qual_t (bool _isConst, bool _isVoltl) : isConst(_isConst), isVoltl(_isVoltl) { }

/**************************************************/
/****************** class "Type" ******************/
/**************************************************/

grp_t Type::grp() { return NONE_G; }

string Type::_str() { return ""; }

/**************************************************/
/****************** class "Base" ******************/
/**************************************************/

Base::Base() { }

Base::Base(base_t _base) : base(_base) { }

grp_t Base::grp() { return BASE_G; }

/*************************************************/
/****************** class "Ptr" ******************/
/*************************************************/

Ptr::Ptr(class Type *_pt) : pt(_pt) { ptrs.push_back(qual_t()); }

Ptr::Ptr(class Type *_pt, bool _isConst, bool _isVoltl) : pt(_pt) { ptrs.push_back(qual_t(_isConst, _isVoltl)); }

grp_t Ptr::grp() { return PTR_G; };

void Ptr::newPtr() { ptrs.push_back(qual_t()); }

void Ptr::newPtr(struct _qual_t q) { ptrs.push_back(qual_t(q)); }

void Ptr::newPtr(bool _isConst, bool _isVoltl) { ptrs.push_back(qual_t(_isConst, _isVoltl)); }

/*************************************************/
/****************** class "Arr" ******************/
/*************************************************/

Arr::Arr(class Type *_item) : item(_item) { dims.push_back(NULL); }

Arr::Arr(class Type *_item, struct _node_t *_evalNode) : item(_item) { dims.push_back(_evalNode); }

void Arr::newDim() { dims.push_back(NULL); }

void Arr::newDim(struct _node_t *_evalNode) { dims.push_back(_evalNode); }

grp_t Arr::grp() { return ARR_G; }

/**************************************************/
/****************** class "Func" ******************/
/**************************************************/

Func::Func(class Type *_retType) : retType(_retType) { }

void Func::newParam(class Type *t) { params.push_back(t); }

grp_t Func::grp() { return FUNC_G; }


/****************************************************/
/****************** string methods ******************/
/****************************************************/

static void replace(string &str, const string &a, const string &b) {
    // Replace the instance of a (in str) with b. Only one instance guaranteed.
    int _start = str.find(a);
    if (_start != string::npos) str.replace(_start, a.size(), b);
}

static void rm(string &str, const string &a) { // remove all instances of a in str.
    int la = a.size(), _start;
    while ((_start = str.find(a)) != string::npos) str.replace(_start, la, "");
}

string str(class Type *t) {
    if (!t) return "";

    if (t->isErr) return "error_type";

    string str;

    switch (t->strg) {
        case AUTO_S : str += "auto "; break;
        case EXTERN_S : str += "extern "; break;
        case REGISTER_S : str += "register "; break;
        case STATIC_S : str += "static "; break;
        case TYPEDEF_S : str += "typedef "; break;
    }

    str += t->_str();
    
    rm(str, "<p>"); rm(str, "<ab>"); rm(str, "<ad>");
    rm(str, "<fb>"); rm(str, "<fp>");
    rm(str, "  ");
    
    // undesired : "( ", " )", add more ...
    int _start;
    while ((_start = str.find("( ")) != string::npos) str.replace(_start, 2, "(");
    while ((_start = str.find(" )")) != string::npos) str.replace(_start, 2, ")");
    while ((_start = str.find(" ,")) != string::npos) str.replace(_start, 2, ",");
    
    // NOTE: Assured that all double spaces ("  ") removed.
    if (str[0] == ' ') str.erase(0, 1);
    if (str.back() == ' ') str.pop_back();
    
    return str;
}

string Base::_str() {
    vector<string> sv;
    if (isConst) sv.push_back("const");
    if (isVoltl) sv.push_back("volatile");

    switch (sign) {
        case SIGNED_X : sv.push_back("signed"); break;
        case UNSIGNED_X : sv.push_back("unsigned"); break;
    }
    string tmp; int _l;
    switch (base) {
        case VOID_B : sv.push_back("void"); break;
        case CHAR_B : sv.push_back("char"); break;
        case SHORT_B : sv.push_back("short int"); break;
        case INT_B : sv.push_back("int"); break;
        case FLOAT_B : sv.push_back("float"); break;
        case LONG_B : sv.push_back("long int"); break;
        case LONG_LONG_B : sv.push_back("long long int"); break;
        case DOUBLE_B : sv.push_back("double"); break;
        case LONG_DOUBLE_B : sv.push_back("long double"); break;
        // case ENUM_B : sv.push_back("enum"); break;
        case STRUCT_B : case UNION_B :
            tmp = tmp + ((base == STRUCT_B) ? "struct " : "union ") + subDef->name;
            if (subDef->syms.size()) {
                tmp += " { ";
                _l = subDef->syms.size();
                for (int i = 0; i < _l; i++) {
                    if (i) tmp += ", ";
                    tmp += subDef->syms[i]->name + " [" + str(subDef->syms[i]->type) + "]";
                }
                tmp += " }";
            } else tmp += " <not yet defined>";


            sv.push_back(tmp);
            break;
        case ELLIPSIS_B : sv.push_back("..."); break;
    }
    string s; int l = sv.size();
    for (int i = 0; i < l; i++) s = s + sv[i] + " ";
    return s + "<p><ab><ad><fb><fp>"; // return according to expectations of others
}

string Ptr::_str() {
    if (!pt) return "";

    string p; int l = ptrs.size();
    for (int i = 0; i < l; i++) {
        p += "*";
        if (ptrs[i].isConst) p += " const ";
        if (ptrs[i].isVoltl) p += " volatile ";
    } // p is the string to replace <p> in incoming string.

    p += "<ab><ad><fb><fp>";
    if (pt->grp() == ARR_G) { p.insert(0, "("); p.push_back(')'); }
    
    string s = pt->_str();
    replace(s, "<p>", p);

    return s; // return according to expectations of others
}

std::string Arr::_str() {
    if (!item) return "";

    string d; int l = dims.size();
    for (int i = 0; i < l; i++) {
        int *x = eval(dims[i]);
        // try getting actual number from dims' nodes, if possible.
        if (x) d += "[" + to_string(*x) + "]";
        else d += "[]";
    }

    string s = item->_str();

    rm(s, "<p>"); rm(s, "<fb>"); rm(s, "<fp>");
    replace(s, "<ab>", "<p>");
    replace(s, "<ad>", d);

    return s; // return according to expectaton of others
}

std::string Func::_str() {
    if (!retType) return "";

    string r = "("; int l = params.size();
    for (int i = 0; i < l; i++) {
        if (i > 0) r += ", "; r += params[i] ? (params[i]->_str()) : "<unknown>";
    }
    r += ")";

    string s = retType->_str();
    rm(s, "<p>"); rm(s, "<ab>"); rm(s, "<ad>");
    replace(s, "<fb>", "(<p>)");
    replace(s, "<fp>", r);

    return s; // return according to expectation of others
}

/*************************************************/
/****************** CLONE MAKER ******************/
/*************************************************/

class Type *clone(class Type *t) {
    if (!t) return NULL;
    Base *b; Ptr *p; Arr *a; Func *f;
    switch (t->grp()) {
        case BASE_G :
            b = new Base(); *b = *((Base *) t); return b;

        case PTR_G :
            p = new Ptr(NULL); *p = *((Ptr *) t); p->pt = clone(((Ptr*) t)->pt); return p;

        case ARR_G :
            a = new Arr(NULL); *a = *((Arr *) t); a->item = clone(((Arr *) t)->item); return a;

        case FUNC_G :
            f = new Func(NULL); *f = *((Func *) t); f->retType = clone(((Func *) t)->retType);
            int l = ((Func *) t)->params.size();
            f->params.clear();
            for (int i = 0; i < l; i++) f->params.push_back(clone(((Func *) t)->params[i]));
            return f;
    }
    Type *c = new Type(); *c = *t; return c;
}


/*************************************************/
/****************** TAIL FINDER ******************/
/*************************************************/

class Type *tail(class Type *t) { // to get tail (last in linked list)
    if (!t) return NULL;
    switch (t->grp()) {
        case BASE_G : return t;
        case  PTR_G : if (((Ptr *)t)->pt) return tail(((Ptr *)t)->pt); else return t;
        case  ARR_G : if (((Arr *)t)->item) return tail(((Arr *)t)->item); else return t;
        case FUNC_G : if (((Func *)t)->retType) return tail(((Func *)t)->retType); else return t;
    }
    return t;
}

class Type *last(class Type *t, grp_t g) { // get last instance of grp type 'g' for linked list 't'. Return NULL is doesn't exist.
    if (!t) return NULL;
    Type *found = NULL;
    while (t) {
        grp_t cg = t->grp();
        if (cg == g) found = t;
        switch (cg) { // now get next of t
            case NONE_G : case BASE_G : t = NULL; break;
            case  PTR_G : t = ((Ptr *) t)->pt; break;
            case  ARR_G : t = ((Arr *) t)->item; break;
            case FUNC_G : t = ((Func *) t)->retType; break;
        }
    }
    return found;
}

void heir(class Type* t) {
    bool isErr = false;
    while (t) {
        isErr |= t->isErr;
        switch (t->grp()) {
            case NONE_G : cout << "NONE_G"; t = NULL; break;
            case BASE_G : cout << "BASE_G"; t = NULL; break;
            case  PTR_G : cout <<  "PTR_G"; t = ((Ptr *) t)->pt; break;
            case  ARR_G : cout <<  "ARR_G"; t = ((Arr *) t)->item; break;
            case FUNC_G : cout << "FUNC_G"; t = ((Func *) t)->retType; break;
        }
        if (t) cout << " >> ";
    }
    if (isErr) cout << _FORE_RED_ << " [ERROR]" << _C_NONE_;
    cout << endl;
}

class Type *unify(class Type *t1, class Type *t2) { // decl_specs with declarator
    if (!(t1 || t2)) return NULL;
    if (!t2) return t1; // simple case (int x;)
    if (!t1) return NULL;
    Type *tt = tail(t2);
    switch (tt->grp()) {
        case  PTR_G : ((Ptr *)tt)->pt = clone(t1); break;
        case  ARR_G : ((Arr *)tt)->item = clone(t1); break;
        case FUNC_G : ((Func*)tt)->retType = clone(t1); break;
    }
    t2->isErr |= t1->isErr; t2->strg = t1->strg;
    return t2;
}

bool extMatch(class Type *prev, class Type *curr) { // match under "extern"
    if (!prev || !curr) return false;
    prev = clone(prev); curr = clone(curr);
    prev->strg = NONE_S; curr->strg = NONE_S;
    return tMatch(prev, curr);
}

bool tMatch(class Type *prev, class Type *curr) { // "EXACTLY" matches two types
    if (!prev || !curr) return false;
    if (prev->isErr || curr->isErr) return false;

    while (prev || curr) {
        if (!prev || !curr) return false;
        if (prev->isErr || curr->isErr) return false;
        grp_t g = prev->grp();
        if (g != curr->grp()) return false;
        Base *b1, *b2; Ptr *p1, *p2; Arr *a1, *a2; Func *f1, *f2; int l1, l2;
        switch (g) {
            case NONE_G : prev = curr = NULL; break;
            
            case BASE_G : b1 = (Base *)prev; b2 = (Base *)curr;
                if ( (b1->base != b2->base) || (b1->sign != b2->sign) || (b1->strg != b2->strg)
                || (b1->isConst != b2->isConst) || (b1->isVoltl != b2->isVoltl)) return false;
                prev = curr = NULL; break;
            
            case  PTR_G : p1 = (Ptr *)prev; p2 = (Ptr *)curr;
                l1 = p1->ptrs.size(); l2 = p2->ptrs.size();
                if (l1 != l2) return false;
                for (int i = 0; i < l1; i++) {
                    qual_t q1 = p1->ptrs[i], q2 = p2->ptrs[i];
                    if ((q1.isConst != q2.isConst) || (q1.isVoltl != q2.isVoltl)) return false;
                }
                prev = p1->pt; curr = p2->pt; break;

            case  ARR_G : a1 = (Arr *)prev; a2 = (Arr *)curr;
                l1 = a1->dims.size(); l2 = a2->dims.size();
                if (l1 != l2) return false;
                // check that array bounds must be equal - later
                prev = ((Arr *)prev)->item; curr = ((Arr *)curr)->item; break;

            case FUNC_G : f1 = (Func*)prev; f2 = (Func*)curr;
                l1 = f1->params.size(); l2 = f2->params.size();
                if (l1 != l2) return false;
                for (int i = 0; i < l1; i++) if (!tMatch(f1->params[i], f2->params[i])) return false;
                prev = f1->retType; curr = f2->retType; break;
        }
    }
    return true;
}

bool checkArrDims(class Type *t) { // recursively check that all array bounds must be present
    if (!t) return true;
    Arr *a;
    switch (t->grp()) {
        case NONE_G : case BASE_G : return true;
        case PTR_G : t = ((Ptr *)t)->pt; break;
        case FUNC_G : t = ((Func *)t)->retType; break;
        case ARR_G : a = (Arr *)t;
            if (a->dims[0] == NULL) return false;
        
            t = a->item; break;
    }
    return true;
}


bool impCast(class Type *from, class Type *to) { // implicit type-casting
    if (!from || !to) return false;
    if (from->isErr || to->isErr) return true;
    grp_t gf = from->grp(), gt = to->grp();
    Base *bf = (Base *)from, *bt = (Base *)to;
    Ptr *pf = (Ptr *)from, *pt = (Ptr *)to;
    Arr *af = (Arr *)from, *at = (Arr *)to;
    Func *ff = (Func *)from, *ft = (Func *)to;
    
    if( gt == FUNC_G ) return false;
    if (gt == BASE_G && gf == BASE_G && bt->base == bf->base) {
        if(bt->base == STRUCT_B || bt->base == UNION_B) {
            if(bt->subDef->name != bf->subDef->name) {
                // if structs are of different name
                return false;
            }
            auto &st1 = bt->subDef->syms, &st2 = bf->subDef->syms;
            int l1 = st1.size(), l2 = st2.size();
            if(l1!=l2) return false;
            for(int i = 0; i <l1; i++) {
                if( st1[i]->name != st2[i]->name || 
                    !tMatch(st1[i]->type, st2[i]->type)) return false;
            }
            return true;
        }
    }
    if( gt == BASE_G && ( bt->base == VOID_B || bt->base == STRUCT_B || bt->base == UNION_B ) ) return false;
    if( gf == BASE_G && ( bf->base == VOID_B || bf->base == STRUCT_B || bf->base == UNION_B ) ) return false;
    
    if (gt == PTR_G && (pt->pt->grp() == BASE_G) && (((Base *)(pt->pt))->base == VOID_B)) {
        if (gf == PTR_G || gf == ARR_G || gf == FUNC_G) return true; // any pointer/array/function is convertible to void *
    }
    
    if (gf == BASE_G) {
        if (gt == BASE_G) return true;
        else {
            if (gt == ARR_G) return false;
            if (priority1[bf->base] > priority1[LONG_LONG_B]) return false;
            return true;
        }
    } else { // from is ARR/PTR/FUNC
        if (gt == BASE_G) { // treating "from" as a pointer here
            if (priority1[bt->base] > priority1[LONG_LONG_B]) return false;
            return true;
        } else { // to is ARR/PTR/FUNC
            // ARR to ARR -ok directly
            // ARR to PTR -ok directly
            // ARR to FUNC -ok directly
            // PTR to PTR - ok directly
            // PTR to ARR -ok directly
            // PTR to FUNC -ok directly
            // FUNC to ARR -ok directly
            // FUNC to PTR -ok directly
            // FUNC to FUNC -ok directly
            return true;
            // if (gt != PTR_G) return false;
        }
        return true;
    }

    
    return true;
}

bool expCast(class Type *from, class Type *to) { // explicit type-casting (may differ from implicit version)
    return impCast(from, to);
}


int *eval(struct _node_t *n) { // evaluate bounds for arrays
    if (!n) return NULL;
    bool minus = false;
    if (n->tok == '-') { minus = true; n = n->ch(0); }
    if (n->tok == '+') { n = n->ch(0); }
    // assume n is now an integer or a char
    Type *t = n->type;
    if (n->tok == CONSTANT) {
        base_t bs = ((Base*)t)->base;
        if (bs == INT_B) return new int(stoi(n->label) * (minus ? (-1) : 1));
        else if (bs == CHAR_B) return new int(n->label[0] * (minus ? (-1) : 1));
    }
    return NULL;
}

bool isReal(class Type* t) {
    if(t && !t->isErr && t->grp() == BASE_G){
        Base* b = (Base*) t;
        if(priority1[b->base] >= priority1[FLOAT_B])
            return true;
    }
    return false;
}

bool isChar(class Type* t) {
    if(t && !t->isErr && t->grp() == BASE_G){
        Base* b = (Base*) t;
        if(b->base == CHAR_B)
            return true;
    }
    return false;
}

bool isShort(class Type* t) {
    if(t && !t->isErr && t->grp() == BASE_G){
        Base* b = (Base*) t;
        if(b->base == SHORT_B)
            return true;
    }
    return false;
}

bool isArr(class Type* t) {
    if(t && !t->isErr && t->grp() == ARR_G){
        return true;
    }
    return false;
}

bool isStruct(class Type* t) {
    if(t && !t->isErr && t->grp() == BASE_G){
        Base* b = (Base*) t;
        if(b->base == STRUCT_B)
            return true;
    }
    return false;
}

bool isPtr(class Type* t) {
    if(t && !t->isErr && t->grp() == PTR_G){
        return true;
    }
    return false;
}

short unsigned int getSize(class Type *t) { // implmentation like "sizeof"
    if (!t) return 1;
    Base *b = (Base *) t;
    Arr *a = (Arr *) t;
    int cmpSize = 0; // coumpound type size (struct/union)
    int baseSize, arrSize, *dimSizePtr;
    vector<sym*> *li; int l;
    switch (t->grp()) {
        case BASE_G : switch (b->base) {
            case SHORT_B: return 2;
            case INT_B : case FLOAT_B : case ENUM_B : return 4;
            case LONG_B : case DOUBLE_B : return 4; // TODO: make 8
            case LONG_LONG_B : case LONG_DOUBLE_B : return 8;
            case STRUCT_B :
                li = &(b->subDef->syms); l = li->size();
                for (int i = 0; i < l; i++) {
                    sym* s = (*li)[i];
                    int size = s->size;
                    if(size %4 != 0) size = size - size%4 + 4;
                    s->offset = cmpSize;
                    cmpSize += size;
                    // cout << s->name << "  " << s->size << " "<< s->offset << endl;
                }
                return cmpSize;
            case UNION_B :
                li = &(b->subDef->syms); l = li->size();
                for (int i = 0; i < l; i++) if (cmpSize < (*li)[i]->size) cmpSize = (*li)[i]->size;
                if(cmpSize % 4 != 0) cmpSize = cmpSize - cmpSize%4 + 4;
                return cmpSize;
            default : return 1; // NONE_B, VOID_B, CHAR_B, SHORT_B, ELLIPSIS_B
        }
        break;
        case PTR_G : return 4; break;
        case ARR_G : 
            baseSize = getSize(a->item);
            arrSize = 1;
            for (auto dim: a->dims) {
                if (!dim) continue;
                dimSizePtr = eval(dim);
                if(dimSizePtr) arrSize *= (*dimSizePtr);
            }
            return arrSize*baseSize;
            break;
        case FUNC_G : return 4; // do this like a function pointer
    }
    return 1;
}

void resetOffset(int tok, symtab *def) {
    if (!def) return;
    def->offset = 0;
    int subtract = def->syms[0]->offset;
    for (auto child : def->syms) {
        if (tok == STRUCT) child->offset -= subtract;
        else child->offset = 0; // tok == UNION
    }
}

struct _node_t * str2arr(struct _node_t *n) {
    if (!n) return NULL;
    string s(n->label); int l = s.size(); vector<int> arr; vector<string> strChar;

    for (int i = 0; i < l; i++) {
        if (s[i] == '\\' && ((i + 1) < l)) switch (s[i+1]) { // \t, \n, \b, \r, \", \\, \'
            case 't' : arr.push_back((int)('\t')); strChar.push_back("\t"); i += 1; break;
            case 'n' : arr.push_back((int)('\n')); strChar.push_back("\n"); i += 1; break;
            case 'b' : arr.push_back((int)('\b')); strChar.push_back("\b"); i += 1; break;
            case 'r' : arr.push_back((int)('\r')); strChar.push_back("\r"); i += 1; break;
            case '\"' : arr.push_back((int)('\"')); strChar.push_back("\""); i += 1; break;
            case '\'' : arr.push_back((int)('\'')); strChar.push_back("\'"); i += 1; break;
            case '\\' : arr.push_back((int)('\\')); strChar.push_back("\\"); i += 1; break;
        } else {
            arr.push_back((int)(s[i])); strChar.push_back(string(1, s[i]));
        }
    }
    arr.push_back(0); strChar.push_back("\0"); // NULL termination

    int arrLen = arr.size();
    struct _node_t *nArr = nd(INIT_LIST, "array", n->pos);
    for (int i = 0; i < arrLen; i++) {
        node_t* ch = nd(CONSTANT, (string("\'") + strChar[i] + string("\'")).c_str(), n->pos);
        Base *b = new Base(CHAR_B); b->isConst = true; ch->type = b;
        ch->eval = to_string(arr[i]);
	    op( nArr, 0, 1, ej(ch) );
    }

    return nArr;
}

string char2num(string s) { // convert char to num
    int l = s.size();
    if (l < 2) return to_string((int)('\0'));
    if (s[1] == '\\' && (2 < l)) switch (s[2]) {
        case 't' : return to_string((int)('\t'));
        case 'n' : return to_string((int)('\n'));
        case 'b' : return to_string((int)('\b'));
        case 'r' : return to_string((int)('\r'));
        case '\"' : return to_string((int)('\"'));
        case '\'' : return to_string((int)('\''));
        case '\\' : return to_string((int)('\\'));
    } else return to_string((int)(s[1]));
    return to_string(0);
}



void arrayInit(struct _loc_t eqPos, string arrName, class Arr *lhs, struct _node_t *rhs, vector<int> index) {
    // check that array dimensions match. Each array element must be typecastable, and emit apt. code.
    if (!lhs || !rhs) {
        repErr(eqPos, "something went wrong while initializing array", _FORE_RED_);
        return;
    }

    // a[3][4] = {{1, 2}, {3, 4, 5}, {6, 7, 8, 9}};
    // index = {1, 2}
    
    int is = index.size(), *x = eval(lhs->dims[is - 1]);
    if (x) {
        if (*x != rhs->numChild) repErr(eqPos, "warning: array sizes do not match on LHS and RHS", _FORE_MAGENTA_);
        
        int min = rhs->numChild; if (min > *x) min = *x;

        for (int i = 0; i < min; i++) { // go over all the children (sub-arrays)
            node_t *ch = rhs->ch(i);
            if (ch->tok == INIT_LIST) { // sub-array
                index[is - 1] = i;
                index.push_back(0);
                arrayInit(eqPos, arrName, lhs, ch, index);
                index.pop_back();
            } else { // at the very base : start assignment
                index[is - 1] = i;
                if (!impCast(ch->type, lhs->item)) {
                    repErr(ch->pos, "cannot implicitly typecast from \"" + str(ch->type) + "\" to \"" + str(lhs->item) + "\"", _FORE_RED_);
                }
                string s = arrName;
                int l = index.size();
                for (int j = 0; j < l; j++) s += "[" + to_string(index[j]) + "]";
                // 4 cases of assignment
                string _opr = eps;
                bool realLHS = isReal(lhs->item), realRHS = isReal(ch->type);
                if (realLHS != realRHS) _opr = realLHS ? "int2real" : "real2int";
                emit(s, lhs, _opr, ch->eval, ch->type);
                if (realLHS && realRHS) IRDump.back().eq = "real=";
            }
        }
    }
}

bool isFuncType(class Type* t) {
    if(!t) return false;
    if(t->grp() == FUNC_G) return true;
    if(t->grp() == PTR_G) {
        Ptr * p = (Ptr *) t;
        if(p->ptrs.size() == 1 && p->pt->grp() == FUNC_G)
            return true;
    }
    return false;
}

/************************************************/
/****************** TEST SUITE ******************/
/************************************************/

#ifdef TEST_TYPES_2

#define MAX_TESTNAME_WIDTH 12

void testOut(string testName, string testResult) {
    cout << _C_BOLD_ << _FORE_GREEN_ << setw(MAX_TESTNAME_WIDTH) << testName << " :";
    cout << _C_NONE_ << " \"" << testResult << "\"" << endl;
}

void testBase() { // const unsigned int volatile static;
    Base* b = new Base();
    b->base = INT_B; b->sign = UNSIGNED_X; b->strg = STATIC_S; b->isConst = true; b->isVoltl = true;
    
    Type* t = b;
    if (t->grp() == BASE_G) testOut("base class", str(t));
    testOut("clone_base", str(clone(t)));
}

void testPtr() { // const int ** const *volatile;
    Base *b = new Base(INT_B); b->isConst = true;
    Ptr* p = new Ptr(b); p->newPtr(true, false); p->newPtr(false, true);

    Type* t = p;
    if (t->grp() == PTR_G) testOut("ptr class", str(t));
    testOut("clone_ptr", str(clone(t)));
}

void testArr() { // const double [][][];
    Base *b = new Base(DOUBLE_B); b->isConst = true;
    Arr *a = new Arr(b); a->newDim(NULL); a->newDim();

    Type *t = a;
    if (t->grp() == ARR_G) testOut("arr class", str(t));
    testOut("clone_arr", str(clone(t)));
}

void testFunc() { // void ()(unsigned char, ...);
    Base *b = new Base(VOID_B);
    Base *b1 = new Base(CHAR_B); b1->sign = UNSIGNED_X;
    Base *b2 = new Base(ELLIPSIS_B);
    Func *f = new Func(b); f->newParam(b1); f->newParam(b2);
    
    Type *t = f;
    if (t->grp() == FUNC_G) testOut("func class", str(t));
    testOut("clone_func", str(clone(t)));
}

void testComplex_1() { // int *const (* volatile f)(); // surprisingly, a "const" (pointer or value) can be returned.
    Base *b = new Base(INT_B);
    Ptr *pb = new Ptr(b, true, false);
    Func *f = new Func(pb);
    Ptr *p = new Ptr(f, false, true);

    Type *t = p;
    if (t->grp() == PTR_G) testOut("complex_1", str(t));
    testOut("clone_1", str(clone(t)));
}

void testComplex_2() { // float (*f)[4][5][6];
    Base *b = new Base(FLOAT_B);
    Arr *a = new Arr(b, NULL); a->newDim(); a->newDim(NULL);
    Ptr *p = new Ptr(a);

    Type *t = p;
    if (t->grp() == PTR_G) testOut("complex_2", str(t));
    testOut("clone_2", str(clone(t)));
}

void testComplex_3() { // int *** (*(*)(char))[]
    Base *b = new Base(INT_B); // int
    Ptr *p = new Ptr(b); p->newPtr(); p->newPtr(); // int ***
    Arr *a = new Arr(p); // int*** []
    Ptr *pa = new Ptr(a); // int *** (*) []
    Func *f = new Func(pa); f->newParam(new Base(CHAR_B)); // int *** (*()(char))[]
    Ptr *pf = new Ptr(f); // int *** (*(*)(char))[]

    Type *t = pf;
    if (t->grp() == PTR_G) testOut("complex_3", str(t));
    testOut("clone_3", str(clone(t)));
    heir(t);
}

void testComplex_4() { // int *(*(*(*(*(*(*[])[])[])[])[])[])[]
    Base *b = new Base(INT_B); // int
    Ptr *p1 = new Ptr(b); // int *
    Arr *a1 = new Arr(p1); // int *[]
    Ptr *p2 = new Ptr(a1); // int *(*)[]
    Arr *a2 = new Arr(p2); // int *(*[])[]
    Ptr *p3 = new Ptr(a2); // int *(*(*)[])[]
    Arr *a3 = new Arr(p3); // int *(*(*[])[])[]
    Ptr *p4 = new Ptr(a3); // int *(*(*(*)[])[])[]
    Arr *a4 = new Arr(p4); // int *(*(*(*[])[])[])[]
    Ptr *p5 = new Ptr(a4); // int *(*(*(*(*)[])[])[])[]
    Arr *a5 = new Arr(p5); // int *(*(*(*(*[])[])[])[])[]
    Ptr *p6 = new Ptr(a5); // int *(*(*(*(*(*)[])[])[])[])[]
    Arr *a6 = new Arr(p6); // int *(*(*(*(*(*[])[])[])[])[])[]
    Ptr *p7 = new Ptr(a6); // int *(*(*(*(*(*(*)[])[])[])[])[])[]
    Arr *a7 = new Arr(p7); // int *(*(*(*(*(*(*[])[])[])[])[])[])[]

    Type *t = a7;
    if (t->grp() == ARR_G) testOut("complex_4", str(t));
    testOut("clone_4", str(clone(t)));
}

void testComplex_5() { // int *** (*(*)(char))()
    Base *b = new Base(INT_B); // int
    Ptr *p = new Ptr(b); p->newPtr(); p->newPtr(); // int ***
    Func *f = new Func(p); // int*** ()()
    Ptr *pf = new Ptr(f); // int *** (*) ()
    Func *ff = new Func(pf); ff->newParam(new Base(CHAR_B)); // int *** (*()(char)) ()
    Ptr *pff = new Ptr(ff); // int *** (*(*)(char))()

    Type *t = pff;
    if (t->grp() == PTR_G) testOut("complex_5", str(t));
    testOut("clone_5", str(clone(t)));
}

int main(){
	testBase();
	testPtr();
	testArr();
	testFunc();

    testComplex_1();
    testComplex_2();
    testComplex_3();
    testComplex_4();
    testComplex_5();

	return 0;	
}

#endif
