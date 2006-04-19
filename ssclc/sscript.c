/* SSCL - Symbion Simple Class Library
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
#include <string.h>

// Grammar
SScriptGrammar *sscript_grammar_init(SScriptGrammar *gr, int size)
{
    hash_init(&gr->cmds, size, NULL);
    return gr;
}

void sscript_grammar_done(SScriptGrammar *gr)
{
    hash_done(&gr->cmds);
}

void sscript_grammar_add_cmd(SScriptGrammar *gr, const char *name, SScriptCmdParser parser)
{
    hash_set(&gr->cmds, name, parser);
}

// Block
SScriptBlock *sscript_block_init(SScriptBlock *blk, SScriptGrammar *gr, LexicalAnalyzer *la)
{
    slist_init(&blk->cmds);
    if (la->token==t_lbrace) lexical_analyzer_next(la);
    while (la->token!=t_rbrace && la->token!=t_eos) {
	if (la->token==t_word) {
	    SScriptCmdParser parser=hash_get(&gr->cmds, la->v_str);
	    if (parser) {
		SScriptCmd *cmd;
		lexical_analyzer_next(la);
		cmd=parser(la);
		slist_append(&blk->cmds, cmd);
	    }
	}
	if (la->token==t_semicolon) lexical_analyzer_next(la);
    }
    return blk;
}

void sscript_block_done(SScriptBlock *blk)
{
    SListItem *p;
    for (p=blk->cmds.first; p; p=p->next) {
	SScriptCmd *cmd=(SScriptCmd*)p->data;
	if (cmd->destroy) cmd->destroy(cmd);
	else free(cmd);
    }
    slist_done(&blk->cmds);
}

// Interp
SScriptInterp *sscript_interp_init(SScriptInterp *interp, int size)
{
    hash_init(&interp->var, size, NULL);
    return interp;
}

void sscript_interp_done(SScriptInterp *interp)
{
    hash_foreach(&interp->var, hash_foreach_free_func, NULL);
    hash_done(&interp->var);
}

int sscript_interp_run(SScriptInterp *interp, SScriptBlock *blk)
{
    SListItem *p;
    for (p=blk->cmds.first; p; p=p->next) {
	SScriptCmd *cmd=(SScriptCmd*)p->data;
	cmd->handler(interp, NULL, cmd->data);
    }
    return 0;
}

// Arg
SScriptArg *sscript_arglist_parse(LexicalAnalyzer *la)
{
    Bool error=false;
    SScriptArg arglist[SSCRIPT_MAX_ARGS+1], *ret;
    int n=0;
    if (la->token==t_lparen) lexical_analyzer_next(la);
    while (la->token!=t_rparen && la->token!=t_eos && !error) {
	if (la->token==t_word) {
	    SScriptArg *arg=&arglist[n];
	    arg->name=str_dup(la->v_str);
	    lexical_analyzer_next(la);
	    if (la->token!=t_word) {error=true; continue;}
	    if (!str_cmp(la->v_str, "int")) {
		arg->type=t_int; arg->intval=0;
	    } else if (!str_cmp(la->v_str, "date")) {
		arg->type=t_sqstring; arg->intval=10;
	    } else if (!str_cmp(la->v_str, "char")) {
		arg->type=t_sqstring;
		lexical_analyzer_next(la);
		if (la->token!=t_lbrac) {error=true; continue;}
		lexical_analyzer_next(la);
		if (la->token!=t_int) {error=true; continue;}
		arg->intval=la->v_int;
		lexical_analyzer_next(la);
		if (la->token!=t_rbrac) {error=true; continue;}
	    } else {error=true; continue;}
	    lexical_analyzer_next(la);
	    if (la->token==t_comma) lexical_analyzer_next(la);
	    n++;
	} else error=true;
    }
    if (error) {
	int i;
	for (i=0; i<n; i++) free(arglist[i].name);
	return NULL;
    } else {
	arglist[n].name=NULL;
	ret=malloc((n+1)*sizeof(SScriptArg));
	memcpy(ret, arglist, (n+1)*sizeof(SScriptArg));
	return ret;
    }
}

void sscript_arglist_free(SScriptArg *arglist)
{
    SScriptArg *arg=arglist;
    if (arglist) {
	while (arg->name) {
	    free(arg->name); arg++;
	}
	free(arglist);
    }
}

char *sscript_expr_parse(SScriptInterp *interp, LexicalAnalyzer *la)
{
    char *ret=NULL;
    switch (la->token) {
	case t_sqstring:
	case t_string:
	    ret=str_dup(la->v_str); break;
	case t_word:
	    ret=str_dup(hash_get(&interp->var, la->v_str)); break;
	default:; //ERROR
    }
    lexical_analyzer_next(la);
    return ret;
}

void sscript_arg_parse(SScriptInterp *interp, SScriptArg *arglist, LexicalAnalyzer *la)
{
    Bool error=false;
    int n=0, i;
    char *name, *val, *nval;
    if (la->token!=t_lparen) error=true;
    lexical_analyzer_next(la);
    while ((la->token!=t_oper || str_cmp(la->v_str, "@")) && !error) {
	nval=sscript_expr_parse(interp, la);
	if (arglist && (name=arglist[n].name)) {
	    if ((val=hash_get(&interp->var, name))) {
		hash_delete(&interp->var, name); free(val);
	    }
	    if (nval) hash_set(&interp->var, name, nval);
	    else {error=true; continue;}
	}
	if (la->token==t_comma) lexical_analyzer_next(la);
	n++;
    }
    while (la->token!=t_rparen && !error) {
	if (la->token!=t_oper || str_cmp(la->v_str, "@")) {error=true; continue;}
	lexical_analyzer_next(la);
	if (la->token!=t_word) {error=true; continue;}
	for (i=n; arglist && arglist[i].name && str_cmp(la->v_str, arglist[i].name); i++);
	lexical_analyzer_next(la);
	if (la->token!=t_oper || str_cmp(la->v_str, "=")) {error=true; continue;}
	lexical_analyzer_next(la);
	nval=sscript_expr_parse(interp, la);
	if (arglist && (name=arglist[i].name)) {
	    if ((val=hash_get(&interp->var, name))) {
		hash_delete(&interp->var, name); free(val);
	    }
	    if (nval) hash_set(&interp->var, name, nval);
	    else {error=true; continue;}
	}
	if (la->token==t_comma) lexical_analyzer_next(la);
    }
}
