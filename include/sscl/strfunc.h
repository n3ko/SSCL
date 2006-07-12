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

#ifndef _SSCL_STRFUNC_H
#define _SSCL_STRFUNC_H

#include <stdlib.h>
#include <stdarg.h>

#ifndef NULL
#define NULL ((void*)0)
#endif

#ifndef CNULL
#define CNULL ((char*)0)
#endif

#define BUF_FREE_S(start, ptr, size) ((size)-((ptr)-(start)))
#define BUF_FREE(start, ptr) (sizeof(start)-((ptr)-(start)))
#define BF(start, ptr) (sizeof(start)-((ptr)-(start)))

#ifndef str_cpy
#  define str_cpy(d, s, n) sscl_str_cpy((d), (s), (n))
#endif
#ifndef str_ecpy
#  define str_ecpy(d, s, n) sscl_str_ecpy((d), (s), (n))
#endif
#ifndef str_tcpy
#  define str_tcpy(d, s, n) sscl_str_tcpy((d), (s), (n))
#endif
#ifndef str_cat
#  define str_cat(d, s, n) sscl_str_cat((d), (s), (n))
#endif

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//	String functions
//=============================================================================
extern char *_str_tok_internal_ptr;
extern char *_str_split_internal_ptr;
extern char *_str_cut_internal_ptr;

static inline char *sscl_str_cpy(char *d, const char *s, int n)
{
    if (!d || !s) return d;
    while (*s && n>0) *d++=*s++, n--;
    *d=0;
    return d;
}

static inline char *sscl_str_cpy_u8(char *d, const char *s, int n, int cn)
{
    char *dsav=d;
    if (!d || !s) return d;
    while (*s && n>0 && cn>0) {
	*d++=*s++; n--;
	if (!*s>>6==2) cn--;
    }
    if (*s>>6==2) { // UTF-8 character cut at the end
	while (d>dsav && *(d-1)>>7) d--;
    }
    *d=0;
    return d;
}

static inline char *sscl_str_ecpy(char *d, const char *s, int n)
{
    if (!d || !s) return d;
    while (*s && n>0) {
	if (*s=='\'' || *s=='\\') {
	    if (n>=2) {
		*d++='\\'; *d++=*s++; n-=2;
	    } else n--;
	} else if (*s=='\n') {
	    if (n>=2) {
		*d++='\\'; *d++='n'; s++; n-=2;
	    } else n--;
	} else if (*s=='\t') {
	    if (n>=2) {
		*d++='\\'; *d++='t'; s++; n-=2;
	    } else n--;
	} else *d++=*s++; n--;
    }
    *d=0;
    return d;
}

static inline char *sscl_str_tcpy(char *d, const char *s, int n)
{
    register char c;
    if (!d || !s) return d;
    while ((c=*s++) && n>0) {
	if (c=='\\') {
	    switch (c=*s++) {
		case 'n': c='\n'; break;
		case 't': c='\t'; break;
		case 0:   continue;
	    }
	}
	*d++=c; n--;
    }
    *d=0;
    return d;
}

static inline char *sscl_str_cat(char *d, const char *s, int n)
{
    if (!d || !s) return d;
    while (*d) d++;
    return sscl_str_cpy(d, s, n);
}

static inline char *str_end(char *s)
{
    while (*s) s++;
    return s;
}

static inline int str_cmp(const char *s1, const char *s2)
{
    while (*s1==*s2 && *s1 && *s2) {
	s1++, s2++;
    }
    return *s1-*s2;
}

static inline int str_ncmp(const char *s1, const char *s2, int n)
{
    n--;
    while (*s1==*s2 && *s1 && *s2 && n>0) s1++, s2++, n--;
    return *s1-*s2;
}

static inline char *str_chr(const char *s, const char c)
{
    while (*s!=c && *s) s++;
    if (*s) return (char *)s;
    else return CNULL;
}

static inline int str_chr_cnt(const char *s, const char c)
{
    int res=0;
    while (*s) {
	if (*s==c) res++;
	s++;
    }
    return res;
}

static inline int str_len(const char *s)
{
    register const char *p;
    for (p=s; *p; p++);
    return p-s;
}

static inline int str_len_u8(const char *s)
{
    register int l=0;
    register const char *p;
    for (p=s; *p; p++) if (!*p>>6==2) l++;
    return l;
}

static inline char *str_dup(const char *s)
{
    if (s) {
	register int l=str_len(s);
	register char *d=(char*)malloc(l+1);
	if (d) sscl_str_cpy(d, s, l);
	return d;
    } else return CNULL;
}

static inline void str_chomp(char *s, const char *chset)
{
    register const char *chs=chset?chset:"\r\n";
    register char *p=s+str_len(s)-1;
    while (p>=s && str_chr(chs, *p)) *p--=0;
}

static inline int str_scale_cmp(const char *scale, const char *s1, const char *s2)
{
    while (*s1==*s2 && *s1 && *s2) s1++, s2++;
    return (str_chr(scale, *s1)-scale)-(str_chr(scale, *s2)-scale);
}

static inline int str_scale_ncmp(const char *scale, const char *s1, const char *s2, int n)
{
    while (*s1==*s2 && *s1 && *s2 && n>0) s1++, s2++, n--;
    return (str_chr(scale, *s1)-scale)-(str_chr(scale, *s2)-scale);
}

static inline char *str_split(char *s, const char *delim)
{
    if (s) _str_split_internal_ptr=s;
    if (!_str_split_internal_ptr) return CNULL;
    else s=_str_split_internal_ptr;
    while (*_str_split_internal_ptr && !str_chr(delim,*_str_split_internal_ptr))
	_str_split_internal_ptr++;
    if (*_str_split_internal_ptr) {
	*_str_split_internal_ptr++=0;
	return s;
    }
    _str_split_internal_ptr=CNULL;
    return s;
};

static inline char *str_tok(char *s, const char *delim)
{
    if (!s) s=_str_tok_internal_ptr;
    if (!s) return CNULL;
    while (*s && str_chr(delim, *s))
	s++;
    if (!*s)
	return _str_tok_internal_ptr=CNULL;
    _str_tok_internal_ptr=s;
    while (*_str_tok_internal_ptr && !str_chr(delim, *_str_tok_internal_ptr))
	_str_tok_internal_ptr++;
    if (!*_str_tok_internal_ptr++) return s;
   *_str_tok_internal_ptr++=0;
    return s;
}

// Cuts maximum length characters at the beginning of string s delimited by
// delim. delim[0] is used for separating words in the target string.
static inline char *str_cut(char *dst, char *src, const char *delim, int length)
{
    char *s, *d, *ssav, *dsav, *strt;
    if (!src) src=_str_cut_internal_ptr;
    if (!src) return CNULL;

    while (str_chr(delim, *src)) src++;
    d=dst; strt=s=src;
    while (d-dst<=length+1) {
	ssav=s; dsav=d;
	while (!str_chr(delim, *s) && d-dst<=length && *s) *d++=*s++;
	while (str_chr(delim, *s) && *s) s++;
	*d++=delim[0];
//	printf(": %d '%s'\n", d-dst, ssav);
    }
//    printf("+ %d %d\n", ssav, strt);
    if (ssav==strt) { // Overflow
	*(d-2)=0;
	_str_cut_internal_ptr=s-1;
    } else {
	*dsav=0;
	_str_cut_internal_ptr=ssav;
    }
    return dst;
}

char *str_itoa(char *d, int n, int i, const char fill, int base);

char *str_printv(char *dst, const char *s, int n, va_list args);
char *str_print(char *dst, const char *s, int n,...);

int str_splitup(char **dst, char *s, int n, int alloc);

#ifdef __cplusplus
}
#endif

#endif /* _SSCL_STRFUNC_H */
