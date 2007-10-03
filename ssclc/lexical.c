/* Symbion Language
 * Copyright (C) 2001-2005 Szilard Hajba
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
#include <sscl/ssclc.h>

static LexicalGrammar default_lexical_grammar={
    LA_DEF_OPERCHARS,
    LA_DEF_STR_BEGIN,
    LA_DEF_STR_END
};

static inline int str_get_c_f(LexicalAnalyzer *la)
{
    register int i=*(*(char**)la->data)++;
    return i ? i : EOS;
    la->file_char++;
}

static inline int file_get_c_f(LexicalAnalyzer *la)
{
    int c=getc((FILE*)la->data);
    if (c!='\n') {
	la->file_char++;
    } else {
	la->file_line++;
	la->file_char=1;
    }
    return c;
}

static inline int stream_get_c_f(LexicalAnalyzer *la)
{
    int c=stream_get_c(STREAM(la->data));
    if (c!='\n') {
	la->file_char++;
    } else {
	la->file_line++;
	la->file_char=1;
    }
    return c;
}

int lexical_analyzer_get_pos_line(LexicalAnalyzer *la)
{
    return la->file_line;
}

int lexical_analyzer_get_pos_char(LexicalAnalyzer *la)
{
    return la->file_char;
}

int lexical_analyzer_init(LexicalAnalyzer *la, LexicalGrammar *gr, GetCharFunc *get_c_f,
	void *data, const char *filename, const int buflen)
{
    la->gr=gr ? gr : &default_lexical_grammar;
    la->get_c=get_c_f;
    la->data=data;
    la->v_str_size=buflen ? buflen : SSCL_BUF_LEN;
    la->v_str=malloc(buflen+1);
    la->c=la->get_c(la);
    la->file_name=filename;
    la->file_line=1;
    la->file_char=1;
    lexical_analyzer_next(la);
    return 0;
}

int lexical_analyzer_init_from_str(LexicalAnalyzer *la, LexicalGrammar *gr,
	const char *str, const int buflen)
{
    la->ext=(void*)str;
    return lexical_analyzer_init(la, gr, (GetCharFunc*)&str_get_c_f, &la->ext, "<STR", buflen);
}

int lexical_analyzer_init_from_file(LexicalAnalyzer *la, LexicalGrammar *gr,
	FILE *f, const char *filename, const int buflen)
{
    if (f) return lexical_analyzer_init(la, gr, (GetCharFunc*)&file_get_c_f, f, filename, buflen);
    else return -1;
}

int lexical_analyzer_init_from_stream(LexicalAnalyzer *la, LexicalGrammar *gr,
	Stream *s, const char *filename, const int buflen)
{
    if (s) return lexical_analyzer_init(la, gr, (GetCharFunc*)&stream_get_c_f, s, filename, buflen);
    else return -1;
}

void lexical_analyzer_done(LexicalAnalyzer *la)
{
    free(la->v_str);
}

Token lexical_analyzer_next(LexicalAnalyzer *la)
{
    register int c=la->c;
    char *quote;
    while ((c<33 && c>0) || c=='#') {
	if (c=='#') while (c!='\n' && c) c=la->get_c(la);
	c=la->get_c(la);
    }
    la->v_str_len=0;
    if (('a'<=c && c<='z') || ('A'<=c && c<='Z') || c=='_') {
	while  ((('a'<=c && c<='z') || ('A'<=c && c<='Z') || ('0'<=c && c<='9')
		|| c=='_') && la->v_str_len<la->v_str_size) {
	    la->v_str[la->v_str_len++]=c; c=la->get_c(la);
	}
//	if (la->v_str_len>=la->v_str_size) Error("E-LEX", "WRDLEN", "Word too long");
	la->token=t_word; la->v_str[la->v_str_len]=0;
    } else if ('0'<=c && c<='9') {
	//int base=10;
	while  ('0'<=c && c<='9' && la->v_str_len<la->v_str_size) {
	    la->v_str[la->v_str_len++]=c; c=la->get_c(la);
	}
//	if (la->v_str_len>=la->v_str_size) Error("E-LEX", "NUMLEN", "Number too long");
	la->v_str[la->v_str_len]=0; la->v_int=atoi(la->v_str);
	la->token=t_int;
//    } else if (c=='"' || c=='\'') {
    } else if ((quote=str_chr(la->gr->str_begin, c))) {
	char end=la->gr->str_end[quote-la->gr->str_begin];
	c=la->get_c(la);
	while (c!=end && c && la->v_str_len<la->v_str_size) {
	    if (c=='\\') {
		switch (c=la->get_c(la)) {
		    case 'n': c='\n'; break;
		    case 'r': c='\r'; break;
		    case 0:   continue;
		}
	    }
	    la->v_str[la->v_str_len++]=c; c=la->get_c(la);
	}
	// if (la->v_str_len>=la->v_str_size) Error("E-LEX", "STRLEN", "String too long");
	// if (c==EOS) Error("E-LEX", "STRCLS", "String constant is open");
	c=la->get_c(la); la->v_str[la->v_str_len]=0;
	la->token=*quote;
    } else if (c==EOS) {
	la->token=t_eos;
    } else if (str_chr(la->gr->operchars, c)) {
	while (str_chr(la->gr->operchars, c) && la->v_str_len<la->v_str_size) {
	    la->v_str[la->v_str_len++]=c; c=la->get_c(la);
	}
	// if (la->v_str_len>=la->v_str_size) Error("E-LEX", "OPRLEN", "Operator too long");
	la->v_str[la->v_str_len]=0;
	la->token=t_oper;
    } else switch (c) {
	case '.': la->token=t_dot; c=la->get_c(la); break;
	case ',': la->token=t_comma; c=la->get_c(la); break;
	case ':': la->token=t_colon; c=la->get_c(la); break;
	case ';': la->token=t_semicolon; c=la->get_c(la); break;
	case '(': la->token=t_lparen; c=la->get_c(la); break;
	case ')': la->token=t_rparen; c=la->get_c(la); break;
	case '{': la->token=t_lbrace; c=la->get_c(la); break;
	case '}': la->token=t_rbrace; c=la->get_c(la); break;
	case '[': la->token=t_lbrac; c=la->get_c(la); break;
	case ']': la->token=t_rbrac; c=la->get_c(la); break;
	default: la->token=t_err; c=la->get_c(la);
//	default: Error("E-LEX", 0, "Parse error at or near '%c'", c);
    }
    la->c=c; return la->token;
}

Bool lexical_analyzer_parse_token(LexicalAnalyzer *la, Token tok)
{
    char *msg;
    if (la->token!=tok) {
	switch (tok) {
	    case t_eos: msg="end of stream"; break;
	    case t_word: msg="word"; break;
	    case t_string: msg="\""; break;
	    case t_sqstring: msg="\'"; break;
	    case t_int: msg="integer value"; break;
	    case t_oper: msg="operator"; break;
	    case t_dot: msg="dot"; break;
	    case t_comma: msg="comma"; break;
	    case t_colon: msg="':'"; break;
	    case t_semicolon: msg="';'"; break;
	    case t_lparen: msg="'('"; break;
	    case t_rparen: msg="')'"; break;
	    case t_lbrace: msg="'{'"; break;
	    case t_rbrace: msg="'}'"; break;
	    case t_lbrac: msg="'['"; break;
	    case t_rbrac: msg="']'"; break;
	    case t_err: msg="error"; break;
	}
	//Error("E-LEX", "TOKEXP", "%s expected", msg);
	return true;
    }
    lexical_analyzer_next(la);
    return false;
}

Bool lexical_analyzer_parse_oper(LexicalAnalyzer *la, const char *oper)
{
    if (la->token!=t_oper || str_cmp(la->v_str, oper)) {
	//Error("E-LEX", "EXPOPR", "operator '%s' expected", oper);
	return false;
    }
    lexical_analyzer_next(la);
    return true;
}

Bool lexical_analyzer_parse_word(LexicalAnalyzer *la, const char *word)
{
    if (la->token!=t_word || str_cmp(la->v_str, word)) {
	//Error("E-LEX", "EXPWRD", "word '%s' expected", word);
	return false;
    }
    lexical_analyzer_next(la);
    return true;
}

char *lexical_analyzer_parse_get_word(LexicalAnalyzer *la, char *buf,
	const int n)
{
    if (la->token==t_word) {
	char *ret=str_cpy(buf, la->v_str, n);
	lexical_analyzer_next(la);
	return ret;
    } else return NULL;
}

char *lexical_analyzer_parse_get_string(LexicalAnalyzer *la, char *buf,
	const int n)
{
    if (la->token==t_string || la->token==t_sqstring) {
	char *ret=str_cpy(buf, la->v_str, n);
	lexical_analyzer_next(la);
	return ret;
    } else return NULL;
}

char *lexical_analyzer_parse_get_sstring(LexicalAnalyzer *la, Token token, char *buf,
	const int n)
{
    if (la->token==token) {
	char *ret=str_cpy(buf, la->v_str, n);
	lexical_analyzer_next(la);
	return ret;
    } else return NULL;
}

int lexical_analyzer_get_c(LexicalAnalyzer *la)
{
    int rc=la->c;
    la->c=la->get_c(la);
    return rc;
}
