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

#define NEW(type) ((type *)malloc(sizeof(type)))

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
char *mem_chunk_alloc(MemChunk *mc);
void mem_chunk_free(MemChunk *mc, void *ptr);

//******************
// Memory Allocator
//******************
void mem_alloc_init(unsigned int nchunk, unsigned int *sizes);
void mem_alloc_done(unsigned int nchunk);
void *mem_alloc_heap(unsigned int size);
void mem_free_heap(void *ptr, unsigned int size);
void *mem_realloc_heap(void *ptr, unsigned int oldsize, unsigned int newsize);

#define NEW_H(type) (type*)mem_alloc_heap(sizeof(type))
#define NEW_HV(type, num) (type*)mem_alloc_heap((num)*sizeof(type))
#define FREE_H(ptr, size) mem_free_heap(ptr, size)

#define NEW_A(type) (type*)alloca(sizeof(type))
#define FREE_A(ptr, size)

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
    SListItem *cursor;
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
 
typedef int (*HashFunc)(const char *str);
#define HASH_FUNC(x) ((HashFunc)x)

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
	const char *key;
	const void *data;
    } entry[0];
};

int cstring_hash(const char *str);
Hash *hash_init(Hash *hash, int size, HashFunc hash_func);
void hash_done(Hash *hash);
const void *hash_get(const Hash *hash, const char *key);
void hash_set(Hash *hash, const char *key, const void *data);
const void *hash_delete(const Hash *hash, const char *key);

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
	t_string='"', t_char='\'',
	t_word=0x100, t_int, t_oper
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

void lexical_analyzer_done(LexicalAnalyzer *la);
Token lexical_analyzer_next(LexicalAnalyzer *la);
Bool lexical_analyzer_parse_token(LexicalAnalyzer *la, Token tok);
Bool lexical_analyzer_parse_oper(LexicalAnalyzer *la, const char *oper);
Bool lexical_analyzer_parse_word(LexicalAnalyzer *la, const char *word);
char *lexical_analyzer_parse_get_word(LexicalAnalyzer *la, char *buf,
	const int n);
char *lexical_analyzer_parse_get_string(LexicalAnalyzer *la, char *buf,
	const int n);
int lexical_analyzer_get_c(LexicalAnalyzer *la);


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
