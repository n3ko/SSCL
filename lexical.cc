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

#include <sscl/lexical.h>
#include <sscl/strfunc.h>
#include <sscl/error.h>

#define LA_OPERATOR_CHARS "=<>~!@$%^&*-+|/?"

LexicalAnalyzer::LexicalAnalyzer(const int len=SSCL_BUF_LEN)
{
    v_str_size=len;
    v_str=new char[len+1];
    token=t_bos;
}

LexicalAnalyzer::~LexicalAnalyzer()
{
}

Token LexicalAnalyzer::next()
{
    while ((c<33 && c>0) || c=='#') {
	if (c=='#') while (c!='\n' && c) c=get_c();
	c=get_c();
    }
    v_str_len=0;
    if (('a'<=c && c<='z') || ('A'<=c && c<='Z') || c=='_') {
	while  ((('a'<=c && c<='z') || ('A'<=c && c<='Z') || ('0'<=c && c<='9')
		|| c=='_') && v_str_len<v_str_size) {
	    v_str[v_str_len++]=c; c=get_c();
	}
	if (v_str_len>=v_str_size) throw new Error("E-LEX", "WRDLEN", "Word too long");
	token=t_word; v_str[v_str_len]=0; return token;
    } else if ('0'<=c && c<='9') {
	while  ('0'<=c && c<='9' && v_str_len<v_str_size) {
	    v_str[v_str_len++]=c; c=get_c();
	}
	if (v_str_len>=v_str_size) throw new Error("E-LEX", "NUMLEN", "Number too long");
	v_str[v_str_len]=0; v_int=atoi(v_str);
	token=t_int;
    } else if (c=='"' || c=='\'') {
	char end=c;
	c=get_c();
	while (c!=end && c && v_str_len<v_str_size) {
//	    fprintf(stderr, "%d ", c);
	    if (c=='\\') {
		switch (c=get_c()) {
		    case 'n': c='\n'; break;
		    case 'r': c='\r'; break;
		    case 0:   continue;
		}
	    }
	    v_str[v_str_len++]=c; c=get_c();
	}
	if (v_str_len>=v_str_size) throw new Error("E-LEX", "STRLEN", "String too long");
	if (!c) throw new Error("E-LEX", "STRCLS", "String constant is open");
	c=get_c(); v_str[v_str_len]=0;
	token=t_string;
    } else if (c==EOF) {
	token=t_eos;
    } else if (strchr(LA_OPERATOR_CHARS, c)) {
	while (strchr(LA_OPERATOR_CHARS, c) && v_str_len<v_str_size) {
	    v_str[v_str_len++]=c; c=get_c();
	}
	if (v_str_len>=v_str_size) throw new Error("E-LEX", "OPRLEN", "Operator too long");
	v_str[v_str_len]=0;
	token=t_oper;
    } else switch (c) {
	case '.': token=t_dot; c=get_c(); break;
	case ',': token=t_comma; c=get_c(); break;
	case ':': token=t_colon; c=get_c(); break;
	case ';': token=t_semicolon; c=get_c(); break;
	case '(': token=t_opensbr; c=get_c(); break;
	case ')': token=t_closesbr; c=get_c(); break;
	case '{': token=t_openbr; c=get_c(); break;
	case '}': token=t_closebr; c=get_c(); break;
	case '[': token=t_opensqbr; c=get_c(); break;
	case ']': token=t_closesqbr; c=get_c(); break;
	default: throw new Error("E-LEX", 0, "Parse error at or near '%c'", c);
    }
    return token;
}

void LexicalAnalyzer::parse_token(Token tok)
{
    char *msg;
    if (next()!=tok) {
	switch (tok) {
	    case t_bos: msg=""; break;
	    case t_eos: msg="end of stream"; break;

	    case t_word: msg="word"; break;
	    case t_string: msg="string value"; break;
	    case t_int: msg="integer value"; break;
	    case t_oper: msg="operator"; break;

	    case t_dot: msg="dot"; break;
	    case t_comma: msg="comma"; break;
	    case t_colon: msg="':'"; break;
	    case t_semicolon: msg="';'"; break;
	    case t_opensbr: msg="'('"; break;
	    case t_closesbr: msg="')'"; break;
	    case t_openbr: msg="'{'"; break;
	    case t_closebr: msg="'}'"; break;
	    case t_opensqbr: msg="'['"; break;
	    case t_closesqbr: msg="']'"; break;
	}
	throw new Error("E-LEX", "TOKEXP", "%s expected", msg);
    }
}

void LexicalAnalyzer::parse_oper(const char *oper)
{
    if (next()!=t_oper || str_cmp(v_str, oper))
	throw new Error("E-LEX", "EXPOPR", "operator '%s' expected", oper);
}

void LexicalAnalyzer::parse_word(const char *word)
{
    if (next()!=t_word || str_cmp(v_str, word))
	throw new Error("E-LEX", "EXPWRD", "word '%s' expected", word);
}

char *LexicalAnalyzer::parse_get_word(char *buf, const int n)
{
    parse_token(t_word);
    return str_cpy(buf, s(), n);
}

char *LexicalAnalyzer::parse_get_string(char *buf, const int n)
{
    parse_token(t_string);
    return str_cpy(buf, s(), n);
}

StrLexicalAnalyzer::StrLexicalAnalyzer(const char *src, const int len=SSCL_BUF_LEN):
	LexicalAnalyzer(len)
{
    ptr=src; c=get_c();
}

int StrLexicalAnalyzer::get_c()
{
    int ret=*ptr++;
    return ret?ret:EOF;
}

int FileLexicalAnalyzer::get_c()
{
    return getc(file);
}
