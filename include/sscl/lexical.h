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
#include <stdio.h>

typedef enum {t_bos, t_eos, t_word, t_string, t_int, t_oper,
	t_dot, t_comma, /*t_slash,*/ t_colon, t_semicolon, /*t_star,*/
	t_opensbr, t_closesbr, t_openbr, t_closebr, t_opensqbr, t_closesqbr
} Token;
typedef enum {la_string, la_file} LexicalAnalyzerType;
class LexicalAnalyzer {
    public:
	LexicalAnalyzer(const int len=SSCL_BUF_LEN);
	virtual ~LexicalAnalyzer();
	virtual Token next();
	virtual Token get_token() {return token;};
	virtual char *s() {return v_str;};
	virtual int i() {return v_int;};
	virtual void parse_token(Token token);
	virtual void parse_oper(const char *oper);
	virtual void parse_word(const char *word);
	virtual char *parse_get_word(char *buf, const int n);
	virtual char *parse_get_string(char *buf, const int n);
	Token operator++() {return next();};
	Token operator++(int) {Token t=token; next(); return t;};
	Token operator*() {return get_token();};
    protected:
	virtual int get_c()=0;
	Token token;
	char c;			// preread cache
	int v_int;
	char *v_str;
	int v_str_len;
	int v_str_size;
};

class StrLexicalAnalyzer: public LexicalAnalyzer {
    public:
	StrLexicalAnalyzer(const char *src, const int len=SSCL_BUF_LEN);
	char *get_string();
    protected:
	virtual int get_c();
	virtual int get_c_wait() {return get_c();};
	const char *ptr;
};

class FileLexicalAnalyzer: public LexicalAnalyzer {
    public:
	FileLexicalAnalyzer(FILE *fil) {
	    file=fil; c=get_c();
	};
	char *get_string();
    protected:
	virtual int get_c();
	virtual int get_c_wait() {return get_c();};
	FILE *file;
};

//static inline void parse_error(const char *text) {
//    throw new Error("F-LEX", "EPARSE", text);
//}

#endif /* _SSCL_LEXICAL_H */
