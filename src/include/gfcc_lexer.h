// AUM SHREEGANESHAAYA NAMAH|| (DIETY INVOCATION)
#ifndef __GFCC_LEXER__
#define __GFCC_LEXER__

#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>

#include <gfcc_meta.h>
#include <types2.h>

// TAB LENGTH
#define TAB_LEN 4 // TODO: must change later

// ERROR CODES
#define E_TOO_FEW_ARGS		(-1)
#define E_INV_OPTION		(-2)
#define E_O_FLAG_TWICE		(-3)
#define E_NUM_IO_UNEQUAL	(-4)
#define E_NO_FILES			(-5)
#define E_TAB_LEN			(-6)
#define E_MULT_DECL			(-7)
#define E_NO_OUT_REQS    (-8)
#define E_NO_LIB_REQS    (-9)

#define OUT_TOK        (0x01)
#define OUT_AST        (0x02)
#define OUT_SYM        (0x04)
#define OUT_3AC        (0x08)
#define OUT_ASM        (0x10)
#define LIB_MATH        (0x1)
#define LIB_TYPO        (0x2)
#define LIB_STD         (0x4)
#define LIB_STRING      (0x8)
#define LIB_BASIC      (0xff) // mainly NULL ptr
// #define LIB_FILE        ??
// #define LIB_TIME        ??


#define UNINIT				(-10)
#define INIT				(-11)
#define FUNC_CALL			(-12) // () is function call
#define SUBSCRIPT			(-13) // [] is subscript operator
#define DEREF				(-14) //  * is dereference operator
#define ARG_EXPR_LIST		(-15) // comma separated expression-arguments passed to a function call
#define ENUM_LIST			(-16) // comma separated enum-values
#define TYPE_QUAL_LIST		(-17) // whitespace separated type qualifiers (strings of [const,volatile])
#define PARAM_TYPE_LIST		(-18) // comma separated parameter type list (for declaring function pointers)
#define ID_LIST				(-19) // comma separated identifier list (for declaring function pointers)
#define EMPTY_BLOCK			(-20) // { }
#define GEN_BLOCK			(-21) // general block (first declarations, then statements)
#define DECL_LIST			(-22) // comma separated declaration list (multiple declaration statements, only specified at a block's beginning)
#define STMT_LIST			(-23) // whitespace separated statement list
#define IF_STMT				(-24) // if statement
#define IF_ELSE_STMT		(-25) // if-else statement
#define TR_UNIT				(-26) // whole translation unit (full file)
#define CAST_EXPR			(-27) // cast expression
#define DECL_SPEC_LIST		(-28) // whitespace separated declaration specifiers (not meaningful for execution/AST)
#define INIT_DECL_LIST		(-29) // comma separated list of variables (that are being declared, maybe initialized)
#define DECLARATION			(-30) // declaration (specifiers, then list of variables)
#define SPEC_QUAL_LIST		(-31) // whitespace separated list of specifier qualifiers
#define STRUCT_DECLN_LIST	(-32) // whitespace separated list of struct declarations
#define STRUCT_DECL			(-33) // struct declaration (qualifier list and declarator list)
#define STRUCT_DECL_LIST	(-34) // whitespace separated list of struct declarations

#define FUNC_PTR			(-35) // function pointer
#define DECLARATOR			(-36) // declarator (general variable, a, b[], c[90], *x, (*func)(), etc.)
#define INIT_LIST			(-37) // Array initializer list
#define DO_WHILE			(-38) // Do while iteration statement
#define STRUCT_MEMBER       (-39) // Used to denote a member in struct/union	
#define ALL_MEMBERS			(-40) // To denote all members of a struct/union as a parent node
#define ABSTRACT_DECLARATOR (-41) // Denote the abstracgt declarations like int *, int [], etc.
#define PARAM_DECL          (-42) // parameter declarations
#define ABST_DCLN           (-43) // () used in abstract declratoins 
#define FUNC_DEF            (-44)

#define TERNARY_EXPR        (-45) // <cond_expr> ? <expr1> : <expr2>

typedef unsigned long long ull_t;

typedef struct _loc_t { // location type
	unsigned int line;
	unsigned int column;
	unsigned int lib;
} loc_t;

typedef struct _token_t {
	struct _loc_t pos;
	int tok;
	std::string lexeme;
	bool bad = false;
	_token_t (struct _loc_t, int, std::string); // constructor
} token_t;

typedef struct _node_t {
	ull_t id;
	int tok; // [ IDENTIFIER | CONSTANT | STRING_LITERAL | some other | -1 ] (else -1, usually for internal nodes)
	const char *label;
	const char *attr;
	struct _node_t *parent;
	int numChild;
	struct _edge_t **edges;
	loc_t pos;
	class Type *type;
	std::vector<unsigned int> truelist, falselist, nextlist, breaklist, contlist, caselist, nxtCase;
	int Stmtbegin = -1;
	std::string eval;
	int offset;
	struct _node_t *ch(int); // getting child without much effort
} node_t;

typedef struct _edge_t {
	struct _node_t *node;
	const char *label;
	const char *attr;
} edge_t;

typedef struct _const_t {
	std::string label;
	bool limitWarn = false; // warning if limit exceeded
	std::string multiWarn = ""; // warning if multiple/incompatible specs
	bool isUnsigned = false;
	bool isLong = false; // in context of reals, means double (by default true)
} const_t;

extern const char* TOKEN_NAME_ARRAY[]; // make this const

extern int column, tab_len, bad_char_seen;

extern loc_t gpos; // global position in current file

extern FILE *yyin, *yyout, *temp_out;

extern char *fileName, *yytext;

extern ull_t currNumNodes;

extern std::ifstream in_file;

extern node_t* AstRoot;

extern std::vector<unsigned int> offsets; // line i starts at offsets[i-1]

extern std::vector<struct _token_t> tokDump;

extern loc_t nonPos; // constant default position

#ifdef COMPLETE
extern const char type_spec_attr[];
extern const char strg_class_attr[];
extern const char type_qual_attr[];
#else
extern const char *type_spec_attr;
extern const char *strg_class_attr;
extern const char *type_qual_attr;
#endif

extern const char
	jump_attr[],
	iter_attr[],
	select_attr[],
	sizeof_attr[],
	empty_attr[],
	file_name_attr[],
	func_call_attr[],
	label_attr[];

void lexUnput(char);

char lexInput(void);

void count(); // count characters for every token encountered

void comment(); // [DO NOT CHANGE NAME] for multi-line comment (MLC)

bool matches(const char*, std::string); // check if first is equal to second

bool matches(const char*, std::string, std::string); // check if first is equal to second or third

void handle_bad_char(); // to handle errors

void update_location(char);

// int yyparse();

// int yywrap();

extern "C" int yylex();

ull_t newNode();

// token_type, label, attr, line, column
node_t* Nd(int, const char*, const char*, loc_t); // attr may be NULL

// token_type, label, line, column
node_t* nd(int, const char*, loc_t); // attr will be passed to Nd as NULL

// child, label, attr
edge_t* Ej(node_t*, const char*, const char*); // label and attr may be NULL

// child
edge_t* ej(node_t*); // label and attr will be passed to mkGenEdge as NULL

// parent, numLeft, numRight, edge_1, edge_2, ...
node_t* op(node_t*, int, int, ...);

void purgeAST(node_t *); // free unrequired data structures

bool accept(node_t *node);

void resetLexer(); // reset global variables

void pushTok(struct _loc_t, int, std::string);

void dumpTok(std::ofstream &, std::vector<struct _token_t> &); // to dump tokens (like Milestone 1)

const_t constParse(std::string, bool); // parse passed constant into SPIM-digestable format.

ull_t maxLimit(bool, bool);

bool intExceeds(ull_t, ull_t, ull_t, bool, bool);

#endif
