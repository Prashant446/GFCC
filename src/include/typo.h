// AUM SHREEGANESHAAYA NAMAH|| (DIETY INVOCATION)
/***********************************************************************/
/************************ NOTE TO THE DEVELOPER ************************/
/***********************************************************************
 * This file is for stuff related to typography on the terminal. This
 * usually includes printing formatted errors, outputs, warnings, etc.
***********************************************************************/
#ifndef __GFCC_TYPO__
#define __GFCC_TYPO__

#include <iostream>
#include <fstream>
#include <sstream>

#include <gfcc_colors.h>
#include <gfcc_lexer.h>

#define SUCC 1
#define WARN 2
#define ERR 3

class msg : public std::stringstream {
	int msg_type = 0;
	public:
		msg();
		msg(int);
		~msg();
};

int yyerror(const char *);

std::string tabExp(std::string &); // carefully replace '\t' by tab_len number of white spaces

// report error
void repErr(loc_t &, std::string, const char *); // very similar to yyerror

void dotStmt(const char*, ...);

void dotNode(std::ofstream &, node_t*);

void dotEdge(std::ofstream &, node_t*, edge_t*);

void AstToDot(std::ofstream &, node_t *);

void lex_err(const char*, ...); // [Deprecated] printf wrapper for colorized output

void lex_warn(const char*, ...); // [Deprecated] printf wrapper for colorized output

#endif
