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

#include <sscl/strfunc.h>

char *_str_tok_internal_ptr;
char *_str_split_internal_ptr;
char *_str_cut_internal_ptr;
char *_str_num_digits="0123456789ABCDEF";

char *str_itoa(char *d, int n, int i, const char fill, int base)
{
    char num[32], *p=num;
//    char *f=;
    if (n>0) {
	if (i<0) {
	    // negative
	    *d++='-'; n--; i=-i;
	} else if (!i) {
	    // zero
	    *d++='0'; n--;
	}
	while (i && n>0) {
	    register int is=i % base;
	    *p++=is<10 ? '0'+is : (is<36 ? 'A'+is-10 : '*'); n--;
	    i/=base;
	}
	while (p>num) {*d++=*--p;};
    }
    *d=0; return d;
}

char *str_printv(char *dst, const char *s, int n, va_list args)
{
    char *p, *d=dst, *d1;
    while (*s && n>0) {
	if (*s=='%') {
	    switch (*++s) {
		case '%': *d++='%', n--; break;
		case 'c': *d++=va_arg(args, int), n--; break;
		case 's': if ((p=va_arg(args, char *))) {
//fprintf(stderr, "str_cpy(%p, '%s', %d)", d, p, n);
			d1=d; d=str_cpy(d, p, n); n-=(d-d1);
//fprintf(stderr, "=%p (%d)\n", d, d-d1);
		    }
		    break;
		case 'e': if ((p=va_arg(args, char *))) {
			d1=d; d=str_ecpy(d, p, n); n-=(d-d1);
		    }
		    break;
		case 'd': d1=d;
			d=str_itoa(d, n, va_arg(args, int), 0, 10); n-=(d-d1);
			break;
		case 'x': d1=d;
			  *d++='0'; *d++='x';
			d=str_itoa(d, n, va_arg(args, int), 0, 16); n-=(d-d1);
			break;
		default:;
//			throw Error("E-STR", "TYPCHR", "Invalid type char '%c'", *s);
	    }
	    s++;
	} else *d++=*s++, n--;
//fprintf(stderr, "n=%d d-dst=%d n-(d-dst)=%d s='%s'\n", n, d-dst, n-(d-dst), s);
    }
    *d=0;
    return d;
}

char *str_print(char *dst, const char *s, int n,...)
{
    char *p;
    va_list args;
    va_start(args, n);
    p=str_printv(dst, s, n, args);
    va_end(args);
    return p;
}

int str_splitup(char **dst, char *s, int n, int alloc)
{
    char *last=s, *d=s, c;
    int i=0;
    while (*s && i<n) {
	switch (*s) {
	    case '|':
		*d=0;
		dst[i]=alloc ? str_dup(last) : last;
		last=++s; d=s;
		i++;
		break;
	    case '\n':
		*d=0; break;
	    case '\\':
		s++;
		switch (c=*s++) {
		    case 'n': c='\n'; break;
		    case 't': c='\t'; break;
		    case 0:   continue;
		}
		*d++=c;
		break;
	    default:
		*d++=*s++;
	}
    }
    *d=0;
    dst[i]=alloc ? str_dup(last) : last;
    i++;
    return i;
}

// dest buf, source, pad type, length in character. buff need at least 1 byte
// larger
char *sscl_str_pad(char *d, const char *s, char type, int l) {
    int fill=(int)l-str_len(s);
    if (fill<0) { str_cpy(d, s, (int)l);}
    else switch(type) {
	case 'l':
	    str_cpy(d, s, l); memset(d+strlen(s), ' ', fill); break;
	case 'r':
	    memset(d, ' ', fill); str_cpy(d+fill, s, l); break;
	case 'c':
	    memset(d, ' ', (int)l); str_cpy(d+fill/2, s, l); break;
	default:
	    l=0;
    }
    d+=l; *d=0;
    return d;
}
// dest buf, source, pad type, length in character, buff size (byte)
char *sscl_str_pad_u8(char *d, const char *s, char type, int l, int n) {
    int fill= (int)l-str_len_u8(s);
    char *end;
    printf("-%s-",s);
    if (fill<0) { end=sscl_str_ncpy_u8(d, s, l, n); fill=0; }
    else switch(type) {
	case 'l':
	    end=sscl_str_cpy(d, s, n-fill); memset(end, ' ', fill);
	    end+=fill; *end=0; break;
	case 'r':
	    memset(d, ' ', fill); end=sscl_str_cpy(d+fill, s, n-fill);
	    break; //*end=0: sscl done it.
	case 'c':
	    memset(d, ' ', fill/2+1); end=sscl_str_ncpy_u8(d+fill/2, s, l, n-fill);
	    memset(end,' ', fill-fill/2); end+= (fill-fill/2);
	    *end=0; break;
	default:
	    end=d; *end=0;
    }
    printf("-%s-\n",d);
    return end;
}
