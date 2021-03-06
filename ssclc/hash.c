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

#define HASH_LIMIT 0.5

unsigned int cstring_hash(const char *str)
{
    register unsigned int h;
    for (h=0; *str; h=h*37+*str++);
    return h;
}

Hash *hash_init(Hash *hash, int size, HashFunc hash_func)
{
    int real_size=2, mask=1, i;

    if (size<2) size=16;
    while (real_size<size) {
	real_size<<=1;
	mask=(mask<<1)+1;
    }
    hash->node=malloc(real_size*sizeof(HashNode*));
    hash->count=0;
    hash->size=real_size;
    hash->mask=mask;
    hash->hash_func=hash_func ? hash_func : &cstring_hash;
    for (i=0; i<real_size; i++) hash->node[i]=NULL;
//    fprintf(stderr, "hash=%p, hash->node=%p\n", hash, hash->node);
    return hash;
}

void hash_done(Hash *hash)
{
    int i, j;
    for (i=0; i<hash->size; i++) {
	HashNode *node=hash->node[i];
	if (node) for (j=0; j<node->count; j++)
	    free(node->entry[j].key);
	if (hash->node[i]) mem_free_heap(hash->node[i], sizeof(HashNode)+sizeof(node->entry[0]));
    }
    free(hash->node);
}

void hash_clean(Hash *hash)
{
    int i, j;
    for (i=0; i<hash->size; i++) {
	HashNode *node=hash->node[i];
	if (node) for (j=0; j<node->count; j++)
	    free(node->entry[j].key);
	if (hash->node[i]) mem_free_heap(hash->node[i], sizeof(HashNode)+sizeof(node->entry[0]));
	hash->node[i]=NULL;
    }
    hash->count=0;
}

static void rebuild_hash_table(Hash *hash)
{
    int i, j;
    HashNode **old_node=hash->node;
    hash_init(hash, hash->size<<1, hash->hash_func);
    for (i=0; i<hash->size>>1; i++) {
	HashNode *n=old_node[i];
	if (n) {
	    for (j=old_node[i]->count-1; j>0; j--) {
		hash_set(hash, old_node[i]->entry[j].key,
			old_node[i]->entry[j].data);
	    }
	    FREE_H(n, sizeof(HashNode)+(old_node[i]->count-1)*sizeof(n->entry[0]));
	}
    }
}

void *hash_get(const Hash *hash, const char *key)
{
    unsigned int i, h=hash->hash_func(key) % hash->size;
    HashNode *node=hash->node[h];
    if (!node) return NULL;
    for (i=0; i<node->count && str_cmp(node->entry[i].key, key); i++);
    if (i<node->count) return node->entry[i].data;
    else return NULL;
}

void hash_set(Hash *hash, const char *key, void *data)
{
    unsigned int i, h=hash->hash_func(key) % hash->size;
    HashNode *node=hash->node[h];
//    fprintf(stderr, "h=%d, hash=%p, hash->node=%p\n", h,hash, hash->node);
//    fprintf(stderr, "hash('%s')=%d (%d)\n", key, h, hash->size);
    if (!node) {
	node=mem_alloc_heap(sizeof(HashNode)+sizeof(node->entry[0]));
	node->count=1;
	node->entry[0].key=str_dup(key);
	node->entry[0].data=data;
	hash->node[h]=node;
	hash->count++;
    } else {
	for (i=0; i<node->count && str_cmp(node->entry[i].key, key); i++);
	if (i<node->count) node->entry[i].data=data;
	else {
	    node=mem_realloc_heap(node,
		    sizeof(HashNode)+(node->count)*sizeof(node->entry[0]),
		    sizeof(HashNode)+(node->count+1)*sizeof(node->entry[0]));
	    node->entry[node->count].key=str_dup(key);
	    node->entry[node->count].data=data;
	    node->count++;
	    hash->node[h]=node;
	    hash->count++;
	}
    }
}

const void *hash_delete(Hash *hash, const char *key)
{
    const void *ret;
    unsigned int i, h=hash->hash_func(key) % hash->size;
    HashNode *node=hash->node[h];
    if (!node) return NULL;
    for (i=0; str_cmp(node->entry[i].key, key) && i<node->count; i++);
    if (i<node->count) {
	int j;
	ret=node->entry[i].data;
	for (j=i+1; j<node->count; j++) node->entry[j-1]=node->entry[j];
	node=mem_realloc_heap(node,
		sizeof(HashNode)+(node->count)*sizeof(node->entry[0]),
		sizeof(HashNode)+(node->count-1)*sizeof(node->entry[0]));
	node->count--;
	hash->node[h]=node;
	hash->count--;
	return ret;
    }
    else return NULL;
}

void hash_foreach(Hash *hash, void (*func)(const char *, void *, void *), void *data)
{
    int i, j;
    for (i=0; i<hash->size; i++) {
	HashNode *node=hash->node[i];
	if (node) for (j=0; j<node->count; j++)
	    func(node->entry[j].key, node->entry[j].data, data);
    }
}

void hash_foreach_free_func(const char *key, void *data, void *d)
{
    free(data);
}

char *hash_print(char *d, const char *s, int n, Hash *hash, HashPrintFunc defprint)
{
    char k[1024], *p, c;
    HashPrintFunc print;
    int i;
    while (*s && n>0) {
	if (*s=='\\') {
	    switch (c=*s++) {
		case 'n': c='\n'; break;
		case 't': c='\t'; break;
		case 0:   continue;
	    }
	} else if (*s=='$') {
	    s++;
	    if (*s=='$') *d++=*s++;
	    else {
		print=defprint;
		if (*s=='\\') { print=HASH_PRINT_FUNC(&sscl_str_ecpy); s++; }
		if (*s=='%') { print=HASH_PRINT_FUNC(&sscl_str_ecpy); s++; }
		if (*s=='=') { print=HASH_PRINT_FUNC(&sscl_str_sqlcpy); s++; }
		for (i=0, p=k; (('A'<=*s && *s<='Z') || ('0'<=*s && *s<='9')
			|| ('a'<=*s && *s<='z') || *s=='_') && BF(k, p);
			i++) *p++=*s++;
		*p=0;
		if (hash) {
		    char *d1=d;
		    if (!print) d=str_ecpy(d, hash_get(hash, k), n);
		    else d=print(d, hash_get(hash, k), n);
		    n-=d-d1;
		}
	    }
	} else *d++=*s++, n--;
    }
    *d=0;
    return d;
}
