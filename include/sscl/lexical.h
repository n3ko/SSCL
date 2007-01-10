/* SSCL - Symbion Simple Class Library
 * Copyright (C) 2001 Szilard Hajba
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _SSCL_LEXICAL_H
#define _SSCL_LEXICAL_H

#include <sscl/config.h>
#include <sscl/ssclc.h>
#include <sscl/stream.h>
#include <stdio.h>

namespace SSCL {

class LexicalAnalyzer {
    public:
//	LexicalAnalyzer(const int len=SSCL_BUF_LEN);
	virtual ~LexicalAnalyzer() {lexical_analyzer_done(&cs);}
	virtual Token next() {return lexical_analyzer_next(&cs);}
	virtual Token get_token() {return cs.token;};
	virtual char *s() {return cs.v_str;};
	virtual int i() {return cs.v_int;};
	virtual void test_token(Token token) {;}
	virtual void test_oper(const char *oper) {;}
	virtual void parse_token(Token token)
		{if (lexical_analyzer_parse_token(&cs, token))
			throw Error("E-LEX", "EXPTOK", "token expected");}
	virtual void parse_oper(const char *oper)
		{if (lexical_analyzer_parse_oper(&cs, oper))
			throw Error("E-LEX", "EXPWRD", "operator %s expected", oper);}
	virtual void parse_word(const char *word)
		{if (lexical_analyzer_parse_word(&cs, word))
			throw Error("E-LEX", "EXPWRD", "word '%s' expected", word);}
	virtual char *parse_get_word(char *buf, const int n)
		{char *ret=lexical_analyzer_parse_get_word(&cs, buf, n);
		if (!ret) throw Error("E-LEX", "EXPWRD", "word expected");
		else return ret;}
	virtual char *parse_get_string(char *buf, const int n)
		{char *ret=lexical_analyzer_parse_get_string(&cs, buf, n);
		if (!ret) throw Error("E-LEX", "EXPSTR", "string expected");
		else return ret;}
	virtual char *parse_get_sstring(char *buf, Token token, const int n)
		{char *ret=lexical_analyzer_parse_get_sstring(&cs, token, buf, n);
		if (!ret) throw Error("E-LEX", "EXPSTR", "string expected");
		else return ret;}
	// Override lexical analyzer
	virtual int get_c() {return lexical_analyzer_get_c(&cs);}
	Token operator++() {return next();};
	Token operator++(int) {Token t=cs.token; next(); return t;};
	Token operator*() {return get_token();};
//    protected:
	::LexicalAnalyzer cs;
};

class StrLexicalAnalyzer: public LexicalAnalyzer {
    public:
	StrLexicalAnalyzer(const char *str, LexicalGrammar *gr=NULL, const int len=SSCL_BUF_LEN)
		{lexical_analyzer_init_from_str(&cs, gr, str, len);}
//	~StrLexicalAnalyzer() {lexical_analyzer_done(&cs);}
	char *get_string();
    protected:
};

/*
class StreamLexicalAnalyzer: public LexicalAnalyzer {
    public:
	StreamLexicalAnalyzer(InStream &s) {
	    is=&s; c=f_get_c(); next();
	};
	char *get_string();
    protected:
	virtual int f_get_c();
	virtual int f_get_c_wait() {return is->get_c_wait();};
	InStream *is;
};
*/

class FileLexicalAnalyzer: public LexicalAnalyzer {
    public:
	FileLexicalAnalyzer(FILE *file, char *filename, LexicalGrammar *gr=NULL, int buflen=SSCL_BUF_LEN) {
	    lexical_analyzer_init_from_file(&cs, gr, file, filename, buflen);
	};
//	char *get_string();
};

//static inline void parse_error(const char *text) {
//    throw new Error("F-LEX", "EPARSE", text);
//}

} /* namespace SSCL */
#endif /* _SSCL_LEXICAL_H */
