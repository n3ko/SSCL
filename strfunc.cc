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

#ifndef _SSCL_STRFUNC_H_
#define _SSCL_STRFUNC_H_

#include <sscl/strfunc.h>
#include <sscl/error.h>

char *_str_tok_internal_ptr;
char *_str_split_internal_ptr;
char *_str_cut_internal_ptr;
char *_str_num_digits="0123456789ABCDEF";

char *str_itoa(char *d, int n, int i, const char fill, int base) {
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
	    *d++=is<10 ? '0'+is : (is<36 ? 'A'+is-10 : '*'); n--;
	    i/=base;
	}
    }
    *d=0; return d;
}

char *str_printv(char *dst, const char *s, int n, va_list args) {
    char *p, *d=dst, *d1;
    while (*s && n>0) {
	if (*s=='%') {
	    switch (*++s) {
		case '%': *d++='%'; break;
		case 'c': *d++=va_arg(args, char); break;
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
			d=str_itoa(d, n, va_arg(args, int), 0); n-=(d-d1);
			break;
		default:
			throw new Error("E-STR", "TYPCHR", "Invalid type char '%c'", *s);
	    }
	    s++;
	} else *d++=*s++, n--;
//fprintf(stderr, "n=%d d-dst=%d n-(d-dst)=%d s='%s'\n", n, d-dst, n-(d-dst), s);
    }
    *d=0;
    return d;
}

char *str_print(char *dst, const char *s, int n,...) {
    char *p;
    va_list args;
    va_start(args, n);
    p=str_printv(dst, s, n, args);
    va_end(args);
    return p;
}

#endif /* _SSCL_STRFUNC_H_ */
