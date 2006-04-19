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
#include <unistd.h>
#include <sys/mman.h>

//#define DEBUG

//*************
// MemChunk
//*************
MemChunk *mem_chunk_init(MemChunk *mc, int atom_size, int atom_per_page, MemChunkAllocator mca)
{
    int ps=sizeof(MemChunkPage)+atom_size*atom_per_page;
    if (mca==mca_mmap) {
	// Round up ps to system page size
	int sys_page_size=sysconf(_SC_PAGESIZE);
	fprintf(stderr, "mem_chunk_init(as=%d app=%d): ps1=%d\n", atom_size, atom_per_page, ps);
	ps=((ps+1)/sys_page_size+1)*sys_page_size;
	atom_per_page=(ps-sizeof(MemChunkPage))/atom_size;
	fprintf(stderr, "mem_chunk_init(): ps2=%d app=%d\n", atom_per_page, ps);
    }
    mc->mca=mca;
    mc->atom_size=atom_size;
    mc->atom_per_page=atom_per_page;
    mc->pagesize=ps;
    mc->freelist=NULL;
    mc->first=mc->free=NULL;
//    mc->first=mc->free=NEW(MemChunkPage);
//    mc->first->nfree=atom_per_page;
    return mc;
}

MemChunk *mem_chunk_new(int atom_size, int atom_per_page, MemChunkAllocator mca)
{
    MemChunk *mc=NEW(MemChunk);
    return mem_chunk_init(mc, atom_size, atom_per_page, mca);
}

void mem_chunk_done(MemChunk *mc)
{
    MemChunkPage *p, *q;
    p=mc->first;
    while (p) {
	q=p;
	p=p->next;
	switch (mc->mca) {
	    case mca_malloc: free(q); break;
	    case mca_mmap: /* FIXME */ break;
	}
    }
}

#define ATOM_ADDR(chunk, page, num) ((void**)((void*)(page)+sizeof(MemChunkPage)+(num)*chunk->atom_size))
static void mem_chunk_extend(MemChunk *mc)
{
    int i;
    MemChunkPage *new;
    switch (mc->mca) {
	case mca_malloc: new=malloc(mc->pagesize); break;
	case mca_mmap: new=mmap(NULL, mc->pagesize, PROT_READ||PROT_WRITE,
			       MAP_PRIVATE||MAP_ANONYMOUS, 0, 0); break;
    }
    new->nfree=mc->atom_per_page;
    new->next=mc->first;
    mc->first=new;
    *ATOM_ADDR(mc, new, 0)=mc->freelist;
    for (i=1; i<mc->atom_per_page; i++) *ATOM_ADDR(mc, new, i)=ATOM_ADDR(mc, new, i-1);
    mc->freelist=ATOM_ADDR(mc, new, mc->atom_per_page-1);
}

char *mem_chunk_alloc(MemChunk *mc)
{
    void *addr;
    if (!mc->freelist) {
	mem_chunk_extend(mc);
    }
    addr=mc->freelist;
    mc->freelist=*(void**)mc->freelist;
    return addr;
}

void mem_chunk_free(MemChunk *mc, void *ptr)
{
    *(void**)ptr=mc->freelist;
    mc->freelist=ptr;
}

int mem_chunk_count_free(MemChunk *mc)
{
    int n=0;
    void *p=mc->freelist;
    while (p) {
	p=*(void**)p; n++;
    }
    return n;
}

int mem_chunk_count_size(MemChunk *mc)
{
    int n=0;
    MemChunkPage *p=mc->first;
    while (p) {
	p=p->next; n++;
    }
    return n*mc->atom_per_page;
}

//******************
// Memory Allocator
//******************
static MemChunk **mem_chunk;
static int mem_chunk_n;

#ifdef DEBUG
typedef struct {
    unsigned int n_alloc;
    unsigned int n_free;
    unsigned int n_alloced;
    unsigned int n_max_alloced;
} *mem_chunk_stat;
#endif

void mem_alloc_stat_reset()
{
#ifdef DEBUG
    int i;
    for (i=0; i<mem_chunk_n; i++) {
	(*mem_chunk_stat)[i].n_alloc=0;
	mem_chunk_stat[i]->n_free=0;
	mem_chunk_stat[i]->n_maxalloced=0;
    }
#endif
}

void mem_alloc_stat_show()
{
#ifdef DEBUG
    int i;
    fprintf(stderr, "Memory statistics:\n");
    for (i=0; i<mem_chunk_n; i++) {
	fprintf(stderr, "  Chunk[%d] (size:%d):\tn_alloc=%d n_free=%d n_maxxalloced=%d\n",
		i, i<<WORD_SIZE_LOG2,
		mem_chunk_stat[i]->n_alloc,
		mem_chunk_stat[i]->n_free,
		mem_chunk_stat[i]->n_maxalloced);
    }
#endif
}

void mem_alloc_init(unsigned int nchunk, unsigned int *sizes)
{
    int i;
    mem_chunk_n=nchunk;
    mem_chunk=malloc(nchunk*sizeof(void*));
#ifdef DEBUG
    mem_chunk_stat=malloc(nchunk*sizeof(*mem_chunk_stat[0]));
#endif
    for (i=0; i<nchunk; i++) {
	if (sizes[i]) {
	    mem_chunk[i]=mem_chunk_new(i<<WORD_SIZE_LOG2, sizes[i], mca_malloc);
	} else mem_chunk[i]=NULL;
#ifdef DEBUG
	mem_chunk_stat[i].n_alloc=0;
	mem_chunk_stat[i].n_free=0;
	mem_chunk_stat[i].n_alloced=0;
	mem_chunk_stat[i].n_max_alloced=0;
#endif
    }
}

void mem_alloc_done()
{
    int i;
    for (i=0; i<mem_chunk_n; i++) {
#ifdef DEBUG
	if (mem_chunk[i]) {
	    fprintf(stderr, "Chunk %d (size %d): free:%d/%d alloc:%d free:%d max_alloc:%d\n", i,
		    i<<WORD_SIZE_LOG2, mem_chunk_count_free(mem_chunk[i]),
		    mem_chunk_count_size(mem_chunk[i]),
		    mem_chunk_stat[i].n_alloc,
		    mem_chunk_stat[i].n_free,
		    mem_chunk_stat[i].n_max_alloced);
//	    mem_chunk_print(mem_chunk[i]);
	}
#endif
    }
}

void *mem_alloc_heap(unsigned int size)
{
    register unsigned int wsize=(size+WORD_SIZE-1)>>WORD_SIZE_LOG2;
    void *ret;
    if (!size) return NULL;
#ifdef DEBUG
    fprintf(stderr, "mem_alloc_heap(%d(%d))\n", size, wsize);
#endif
    if (wsize>=mem_chunk_n || !mem_chunk[wsize]) {
#ifdef DEBUG_MEM
	fprintf(stderr, "    malloc(%d)\n", size);
#endif
	ret=malloc(size);
    } else {
	ret=mem_chunk_alloc(mem_chunk[wsize]);
#ifdef DEBUG
	mem_chunk_stat[wsize].n_alloc++;
	mem_chunk_stat[wsize].n_alloced++;
	if (mem_chunk_stat[wsize].n_alloced>mem_chunk_stat[wsize].n_max_alloced)
	    mem_chunk_stat[wsize].n_max_alloced=mem_chunk_stat[wsize].n_alloced;
#endif
    }
#ifdef DEBUG
    fprintf(stderr, "=%p\n", ret);
#endif
    return ret;
}

void mem_free_heap(void *ptr, unsigned int size)
{
    register unsigned int wsize=(size+WORD_SIZE-1)>>WORD_SIZE_LOG2;
    if (!size) return;
#ifdef DEBUG
    fprintf(stderr, "mem_free_heap(%d(%d))\n", size, wsize);
#endif
    if (wsize>=mem_chunk_n || !mem_chunk[wsize]) return free(ptr);
#ifdef DEBUG
    mem_chunk_stat[wsize].n_free++;
    mem_chunk_stat[wsize].n_alloced--;
#endif
    return mem_chunk_free(mem_chunk[wsize], ptr);
}

void *mem_realloc_heap(void *ptr, unsigned int oldsize, unsigned int newsize)
{
    void *new=mem_alloc_heap(newsize);
    memcpy(new, ptr, oldsize>newsize ? newsize : oldsize);
    mem_free_heap(ptr, oldsize);
    return new;
}

//*************
// StringChunk
//*************
/*
StringChunk *string_chunk_new(int size)
{
    return g_string_chunk_new(size);
}

void string_chunk_free(StringChunk *sc)
{
    g_string_chunk_free(sc);
}

char *string_chunk_add(StringChunk *sc, char *str)
{
    return g_string_chunk_insert(sc, str);
}

char *string_chunk_add_const(StringChunk *sc, const char *str)
{
    return g_string_chunk_insert_const(sc, str);
}

StringChunk *string_chunk_init(StringChunk *sc)
{
    sc->size=size;
    slist_init(&sc->page);
    btree_init(&sc->set);
}
*/
