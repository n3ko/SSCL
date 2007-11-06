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
#ifndef _SSCLC_H
#define _SSCLC_H

#include <stdio.h>
#include <sscl/config.h>
#include <sscl/strfunc.h>

#ifdef __cplusplus
typedef bool Bool;
extern "C" {
#else
typedef enum {false, true} Bool;
#endif

//*************
// MemChunk
//*************
typedef enum {mca_mmap, mca_malloc} MemChunkAllocator;
typedef struct _MemChunk MemChunk;
typedef struct _MemChunkPage MemChunkPage;

struct _MemChunk {
    MemChunkAllocator mca;
    int pagesize;
    int atom_size;
    int atom_per_page;
    MemChunkPage *first;
    MemChunkPage *free;
    void *freelist;
};

struct _MemChunkPage {
    int nfree;
    MemChunkPage *next;
};

MemChunk *mem_chunk_init(MemChunk *mc, int atom_size, int atom_per_page, MemChunkAllocator mca);
MemChunk *mem_chunk_new(int atom_size, int atom_per_page, MemChunkAllocator mca);
void mem_chunk_done(MemChunk *mc);
char *mem_chunk_alloc(MemChunk *mc);
void mem_chunk_free(MemChunk *mc, void *ptr);

//******************
// Memory Allocator
//******************
void mem_alloc_init(unsigned int nchunk, unsigned int *sizes);
void mem_alloc_done();
void *mem_alloc_heap(unsigned int size);
void mem_free_heap(void *ptr, unsigned int size);
void *mem_realloc_heap(void *ptr, unsigned int oldsize, unsigned int newsize);
void mem_alloc_stat_reset();
void mem_alloc_stat_show();

#define NEW(type) ((type *)malloc(sizeof(type)))
#define NEW_H(type) (type*)mem_alloc_heap(sizeof(type))
#define NEW_HV(type, num) (type*)mem_alloc_heap((num)*sizeof(type))
#define FREE_H(ptr, size) mem_free_heap(ptr, size)

#define NEW_A(type) (type*)alloca(sizeof(type))
#define FREE_A(ptr, size)

//*********************
// Time
//*********************
void sscl_sleep();
void sscl_sleep_set(int usec);

//*********************
// List
//*********************
typedef struct _ListItem ListItem;
typedef struct _List List;
typedef struct _SListItem SListItem;
typedef struct _SList SList;
#define LIST(x) ((List *)(x))
#define SLIST(x) ((SList *)(x))

struct _ListItem {
    void *data;
    ListItem *next;
    ListItem *prev;
};
struct _List {
    ListItem *first;
    ListItem *last;
    int num;
};

struct _SListItem {
    void *data;
    SListItem *next;
};
struct _SList {
    SListItem *first;
    SListItem *last;
    int num;
};

List *list_init(List *list);
List *list_new();
void list_done(List *list);
void list_done_with_data(List *list);
void list_free(List *list);
void list_free_with_data(List *list);
void list_append(List *list, void *data);
void list_push(List *list, void *data);
void *list_pop(List *list);
void *list_get(List *list, int num);
void list_foreach(List *list, void (*func)(int, void *));
int list_get_num(List *list);
void list_clear(List *list);
void list_clear_with_data(List *list);

SList *slist_init(SList *list);
SList *slist_new();
void slist_free(SList *list);
void slist_free_with_data(SList *list);
void slist_done(SList *list);
void slist_done_with_data(SList *list);
void slist_append(SList *list, void *data);
void list_push(List *list, void *data);
void *list_pop(List *list);
void *slist_get(SList *list, int num);
void slist_foreach(SList *list, void (*func)(int, void *));
int slist_get_num(SList *list);

//*********************
// Hash
//*********************
typedef struct _Hash Hash;
typedef struct _HashNode HashNode;
 
typedef unsigned int (*HashFunc)(const char *str);
#define HASH_FUNC(x) ((HashFunc)x)
typedef char *(*HashPrintFunc)(char *d, const void *data, int n);

struct _Hash {
    int count;
    int size;
    unsigned int mask;
    HashFunc hash_func;
    HashNode **node;
};

struct _HashNode {
    int count;
    struct {
	char *key;
	void *data;
    } entry[0];
};

unsigned int cstring_hash(const char *str);
Hash *hash_init(Hash *hash, int size, HashFunc hash_func);
void hash_done(Hash *hash);
void *hash_get(const Hash *hash, const char *key);
void hash_set(Hash *hash, const char *key, void *data);
const void *hash_delete(Hash *hash, const char *key);
void hash_foreach(Hash *hash, void (*func)(const char *, void *, void *), void *data);
void hash_foreach_free_func(const char *key, void *data, void *d);
char *hash_print(char *d, const char *s, int n, Hash *hash, HashPrintFunc print);

//*********************
// AVLTree
//*********************

//*********************
// Stream
//*********************
#define EOS -1
typedef struct _Stream Stream;
#define STREAM(x) ((Stream*)x)

struct _Stream {
    int fd;
    int ibufl;
    char *ibuf;
    char *ibegin;
    int icnt;
    char *inl;
    // Representation:
    //	[X][X][X][ ][ ][ ][X][X][X][X]
    //	          ^iend    ^ibegin
};

Stream *stream_init_fd(Stream *s, int fd, const int ilen);
Stream *stream_new_fd(int fd, const int ilen);
Stream *stream_init_file(Stream *s, const char *fn, int flags, const int ilen);
Stream *stream_new_file(const char *fn, int flags, const int ilen);
void stream_done(Stream *s);
void stream_free(Stream *s);
int stream_try_read(Stream *s);
int stream_read(Stream *s, char *buffer, int n);
int stream_get_c(Stream *s);
int stream_get_c_wait(Stream *s);
int stream_get_s(Stream *s, char *buffer, int n);
int stream_get_s_wait(Stream *s, char *buffer, int n);
int stream_put_c(Stream *s, const char c);
int stream_write(Stream *s, const char *buffer, int n);
int stream_put_s(Stream *s, const char *buffer);
int stream_print_v(Stream *s, const char *format, va_list args);
int stream_print(Stream *s, const char *format,...);

//*********************
// Network
//*********************
typedef struct _NetConn NetConn;
typedef struct _NetServer NetServer;
#define NETCONN(x) ((NetConn*)x)
#define NETSERVER(x) ((NetServer*)x)

typedef enum {nf_inet, nf_unix} NetConnFamily;

struct _NetConn {
    struct _Stream _parent;
    NetServer *server;
};

struct _NetServer {
    int fd;
    int clientnum;
    NetConnFamily family;
    char *address;
//    AVLTree clients;
};

NetConn *netconn_init(NetConn *net, const NetConnFamily family, const char *addr,
	const int port, const int buflen);
NetConn *netconn_init_server(NetConn *net, NetServer *server, const int fd, const int buflen);
NetConn *netconn_new(const NetConnFamily family, const char *addr, const int port, const int buflen);
void netconn_done(NetConn *net);
void netconn_free(NetConn *net);
char *netconn_get_ip(NetConn *net);

NetServer *netserver_init(NetServer *serv, const NetConnFamily family, const char *addr,
	const int port, const int max_conn, const int flags);
NetServer *netserver_new(const NetConnFamily family, const char *addr, const int port,
	const int max_conn, const int flags);
void netserver_done(NetServer *serv);
NetConn *netserver_accept(NetServer *serv, NetConn *net, const int flags, const int buflen);
void netserver_remove(NetServer *serv, int s);

//*********************
// Lexical analyzer
//*********************
typedef struct _LexicalGrammar LexicalGrammar;
typedef struct _LexicalAnalyzer LexicalAnalyzer;

typedef int (GetCharFunc)(LexicalAnalyzer *la);

#define LA_DEF_OPERCHARS "=<>~!@$%^&*-+|/?"
#define LA_DEF_STR_BEGIN "\"'"
#define LA_DEF_STR_END "\"'"

typedef enum {t_eos=0,
	t_dot='.', t_comma=',', t_colon=':', t_semicolon=';',
	t_lparen='(', t_rparen=')', t_lbrace='{', t_rbrace='}', t_lbrac='[', t_rbrac=']',
	t_string='"', t_sqstring='\'',
	t_word=0x100, t_int, t_oper, t_err=-1
} Token;
typedef enum {la_string, la_file} LexicalAnalyzerType;

struct _LexicalGrammar {
    char *operchars;	// =<>~!@$%^&*-+|/?
    char *str_begin;	// "'
    char *str_end;	// "'
};

struct _LexicalAnalyzer {
    LexicalGrammar *gr;
    GetCharFunc *get_c;
    void *data;		// extra data for get_c(data) func
    void *ext;		// For example init_from_str() uses it to
			// store the wandering pointer
    Token token;	// token last read
    char c;		// preread cache
    int v_int;		// integer value of current token
    char *v_str;	// string value of current token
    int v_str_len;	// length of string value
    int v_str_size;	// string buffer size

    const char *file_name;	// name of the input file
    int file_line;
    int file_char;
};

int lexical_analyzer_get_pos_line(LexicalAnalyzer *la);
int lexical_analyzer_get_pos_char(LexicalAnalyzer *la);

int lexical_analyzer_init(LexicalAnalyzer *la, LexicalGrammar *gr, GetCharFunc *get_c_f,
	void *data, const char *filename, const int buflen);
int lexical_analyzer_init_from_str(LexicalAnalyzer *la, LexicalGrammar *gr,
	const char *str, const int buflen);
int lexical_analyzer_init_from_file(LexicalAnalyzer *la, LexicalGrammar *gr,
	FILE *f, const char *filename, const int buflen);
int lexical_analyzer_init_from_stream(LexicalAnalyzer *la, LexicalGrammar *gr,
	Stream *s, const char *filename, const int buflen);

void lexical_analyzer_done(LexicalAnalyzer *la);
Token lexical_analyzer_next(LexicalAnalyzer *la);
Bool lexical_analyzer_parse_token(LexicalAnalyzer *la, Token tok);
Bool lexical_analyzer_parse_oper(LexicalAnalyzer *la, const char *oper);
Bool lexical_analyzer_parse_word(LexicalAnalyzer *la, const char *word);
char *lexical_analyzer_parse_get_word(LexicalAnalyzer *la, char *buf,
	const int n);
char *lexical_analyzer_parse_get_string(LexicalAnalyzer *la, char *buf,
	const int n);
char *lexical_analyzer_parse_get_sstring(LexicalAnalyzer *la, Token token, char *buf,
	const int n);
int lexical_analyzer_get_c(LexicalAnalyzer *la);

//*********
// SScript
//*********
typedef struct _SScriptGrammar SScriptGrammar;
typedef struct _SScriptCmd SScriptCmd;
typedef struct _SScriptInterp SScriptInterp;
typedef struct _SScriptBlock SScriptBlock;
typedef struct _SScriptArg SScriptArg;
typedef struct _SScriptArgList SScriptArgList;
typedef SScriptCmd *(*SScriptCmdParser)(LexicalAnalyzer *la);
#define SSCRIPT_CMD_FUNC(x) ((SScriptCmdFunc)x)
typedef int (*SScriptCmdHandler)(SScriptInterp *interp, void *_this, void *cmddata, void *data);
typedef void (*SScriptCmdDestroyHandler)(SScriptCmd *cmd);

struct _SScriptGrammar {
    Hash cmds;			// Hash(SScriptCmdParser)
};

struct _SScriptCmd {
    SScriptCmdHandler handler;
    SScriptCmdDestroyHandler destroy;
    void *data;
};

struct _SScriptBlock {
    SList cmds;			// Hash(SScriptCmd)
};

struct _SScriptInterp {
    Hash var;
};

struct _SScriptArg {
    char *name;
    Token type;
    int intval;
};

SScriptGrammar *sscript_grammar_init(SScriptGrammar *gr, int size);
void sscript_grammar_add_cmd(SScriptGrammar *gr, const char *name, SScriptCmdParser parser);
void sscript_grammar_done(SScriptGrammar *gr);

//SScriptCmd *sscript_cmd_init(SScriptCmd *cmd, SScriptGrammar *gr);
SScriptCmd *sscript_cmd_init(SScriptCmd *cmd, SScriptCmdHandler handler, void *data);
void sscript_cmd_done(SScriptCmd *cmd);

SScriptBlock *sscript_block_init(SScriptBlock *blk, SScriptGrammar *gr, LexicalAnalyzer *la);
void sscript_block_done(SScriptBlock *blk);

SScriptInterp *sscript_interp_init(SScriptInterp *interp, int size);
void sscript_interp_done(SScriptInterp *interp);
int sscript_interp_run(SScriptInterp *interp, SScriptBlock *blk, void *data);

SScriptArg *sscript_arglist_parse(LexicalAnalyzer *la);
void sscript_arglist_free(SScriptArg *arglist);
char *sscript_expr_parse(SScriptInterp *interp, LexicalAnalyzer *la);
int sscript_arg_parse(SScriptInterp *interp, SScriptArg *arglist, LexicalAnalyzer *la);
int sscript_arg_parse_ex(SScriptInterp *interp, LexicalAnalyzer *la);

//*********
// XML
//*********
typedef struct _XMLNode XMLNode;
#define XML_NODE(x) ((XMLNode *)x)
typedef struct _XMLPackage XMLPackage;
#define XML_PACKAGE(x) ((XMLPackage *)x)
typedef struct _XMLHandler XMLHandler;
#define XML_HANDLER(x) ((XMLHandler *)x)

typedef void (*XMLParseFunc)(XMLNode *node);
#define XML_PARSE_FUNC(x) ((XMLParseFunc)x)
typedef void *(*XMLNewFunc)(XMLNode *node, void *parent, Hash *vs);
#define XML_NEW_FUNC(x) ((XMLNewFunc)x)

struct _XMLNode {
    char *ns;
    char *cls;
    Hash *args;
    XMLNode *parent;
    SList *child;
    XMLPackage *pkg;
};

struct _XMLNS {
    char *URI;
    Hash *hash;
};

struct _XMLHandler {
    XMLNewFunc new_func;
    XMLParseFunc parse_func;
};

struct _XMLClassHandler {
    char *ns;
    char *cls;
    Hash *args;
    XMLNode *parent;
    SList *child;
};

/* Not implemented yet, conflict with libxml2.
void xml_package_init(XMLPackage *pkg, int sc_size, int node_num);
XMLPackage *xml_package_new(int sc_size, int node_num);
void xml_package_done(XMLPackage *pkg);

XMLNode *xml_node_new(XMLPackage *pkg, char *ns, char *cls, Hash *arg,
	XMLNode *parent, SList *child);
void xml_node_done(XMLNode *node);
void xml_node_append(XMLNode *node, XMLNode *child);
XMLNode *xml_parse_object(XMLPackage *pkg, LexicalAnalyzer *la);
XMLNode *xml_parse(XMLPackage *pkg, LexicalAnalyzer *la);
XMLNode *xml_parse_str(XMLPackage *pkg, char *str);
XMLNode *xml_parse_file(XMLPackage *pkg, char *filename);

void *xml_eval(XMLNode *node, void *parent, Hash *vs);
char *xml_get_val(char *expr, Hash *vs);
char *xml_get_param_val(Hash *params, char *name, Hash *vs);
Hash *xml_parse_param(Hash *params, Hash *vs);

void *xml_handle_object(XMLNode *node, void *parent, Hash *vs);
void *xml_handle_template(XMLNode *node, void *parent, Hash *vs);
void xml_parse_template(XMLNode *node);

void xml_register_handler(char *cls, XMLNewFunc new_func, XMLParseFunc parse_func);
//void xml_register_handler(char *cls, XMLNewFunc *new_func, XMLAddFunc *add_func);
void xml_init();
*/

//************************************
// Symbion Daemon Tools communication
//************************************

typedef enum { st_stopped='-', st_unrun='U',
	st_preinit='i', st_init='I', st_run='R',
	st_quit='Q', st_postquit='q'} SDTState;
typedef enum { sr_none=' ', sr_start='+', sr_stop='-'} SDTRequest;
typedef enum { sp_none,		// No protocol
		sp_clc,		// Command-line compatibility
		sp_fdc		// Full Daemon Control
	    } SDTProtocol;
typedef enum { sl_err='E', sl_warn='W', sl_not='N', sl_info='I' } SDTLogLevel;

#ifdef __cplusplus
}
#endif

#endif /* _SSCLC_H */
