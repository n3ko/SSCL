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

char *sscl_str_ucpy(char *d, const char *s, int n)
{
    char *hex = "0123456789ABCDEF";
    while (*s && n>0) {
	if (*s == ' ') { *d++='+'; }
	else if (('a' <= *s && *s <= 'z') || ('A' <= *s && *s <= 'Z')
	    || ('0' <= *s && *s <= '9') || *s == '-'
	    || *s == '_' || *s == '.') *d++=*s;
	else {
	    if (n>=3) {
		*d++='%';
		*d++=hex[(*s)/16];
		*d++=hex[*s & 0x0f];
		n-=3;
	    } else n=0;
	}
	s++;
    }
    *d=0;
    return d;

}

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
		case 'q': if ((p=va_arg(args, char *))) {
			d1=d; d=str_sqlcpy(d, p, n); n-=(d-d1);
		    }
		    break;
		case 'd': d1=d;
			d=str_itoa(d, n, va_arg(args, int), 0, 10); n-=(d-d1);
			break;
		case 'x': d1=d;
			  *d++='0'; *d++='x';
			d=str_itoa(d, n, va_arg(args, int), 0, 16); n-=(d-d1);
			break;
		case 'U': if ((p=va_arg(args, char *))) {
			d1=d; d=sscl_str_ucpy(d, p, n); n-=(d-d1);
		    }
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
	    str_cpy(d, s, l); memset(d+str_len(s), ' ', fill); break;
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
//    printf("-%s-",s);
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

int str_utf8_decomp(char **str)
{
    unsigned char **s=(unsigned char **)str;
    int ret;
    if (((*s)[0] & 0x80) == 0) {
	// 0xxxxxxx
	ret=*(*s)++;
    } else if ((((*s)[0] & 0xe0) == 0xc0) && (((*s)[1] & 0xc0) == 0x80)) {
	// 110xxxxx 10xxxxxx
	ret=((*s)[0] & 0x1f )<<6 | ((*s)[1] & 0x3f);
	(*s)+=2;
    } else if ((((*s)[0] & 0xf0) == 0xe0) && (((*s)[1] & 0xc0) == 0x80)
	    && (((*s)[2] & 0xc0) == 0x80)) {
	// 1110xxxx 10xxxxxx 10xxxxxx
	ret=((*s)[0] & 0x0f )<<12
		| ((*s)[1] & 0x3f)<<6
		| ((*s)[2] & 0x3f);
	(*s)+=3;
    } else if ((((*s)[0] & 0xf8) == 0xf0) && (((*s)[1] & 0xc0) == 0x80)
	    && (((*s)[2] & 0xc0) == 0x80) && (((*s)[3] & 0xc0) == 0x80)) {
	// 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
	ret=((*s)[0] & 0x07 )<<18
		| ((*s)[1] & 0x3f)<<12
		| ((*s)[2] & 0x3f)<<6
		| ((*s)[3] & 0x3f);
	(*s)+=4;
    } else if ((((*s)[0] & 0xfc) == 0xf8) && (((*s)[1] & 0xc0) == 0x80)
	    && (((*s)[2] & 0xc0) == 0x80) && (((*s)[3] & 0xc0) == 0x80)
	    && (((*s)[4] & 0xc0) == 0x80)) {
	// 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
	ret=((*s)[0] & 0x03 )<<24
		| ((*s)[1] & 0x3f)<<18
		| ((*s)[2] & 0x3f)<<12
		| ((*s)[3] & 0x3f)<<6
		| ((*s)[4] & 0x3f);
	(*s)+=5;
    } else if ((((*s)[0] & 0xfe) == 0xfc) && (((*s)[1] & 0xc0) == 0x80)
	    && (((*s)[2] & 0xc0) == 0x80) && (((*s)[3] & 0xc0) == 0x80)
	    && (((*s)[4] & 0xc0) == 0x80) && (((*s)[5] & 0xc0) == 0x80)) {
	// 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
	ret=((*s)[0] & 0x01 )<<30
		| ((*s)[1] & 0x3f)<<24
		| ((*s)[2] & 0x3f)<<18
		| ((*s)[3] & 0x3f)<<12
		| ((*s)[4] & 0x3f)<<6
		| ((*s)[5] & 0x3f);
	(*s)+=6;
    } else if (((*s)[0] == 0xfe) && (((*s)[1] & 0xc0) == 0x80)
	    && (((*s)[2] & 0xc0) == 0x80) && (((*s)[3] & 0xc0) == 0x80)
	    && (((*s)[4] & 0xc0) == 0x80) && (((*s)[5] & 0xc0) == 0x80)
	    && (((*s)[6] & 0xc0) == 0x80)) {
	// 11111110 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
	ret=((*s)[1] & 0x03)<<30
		| ((*s)[2] & 0x3f)<<24
		| ((*s)[3] & 0x3f)<<18
		| ((*s)[4] & 0x3f)<<12
		| ((*s)[5] & 0x3f)<<6
		| ((*s)[6] & 0x3f);
	(*s)+=7;
    } else {
	// Error
	printf("Error in UTF-8 sequence\n");
	ret=1;
    }
    return ret;
}

#include <stdio.h>
Locale *locale_init(Locale *loc, char *localedef)
{
    char *s=localedef+8, *sav; // Skip 'lang:hu:
    unsigned int min=0xffffff, max=0, i;
    loc->bitshift=*s++-'0'; // FIXME
    s++; // Skip ':'
    sav=s;
    while (*s) {
	int from=str_utf8_decomp(&s);
	if (from<min) min=from;
	if (from>max) max=from;
	if (*s) {
	    str_utf8_decomp(&s);
	    while (*s && *s!=',') s++;
	    s++;
	}
    }
//    fprintf(stderr, "min: %08x   max: %08x\n", min, max);
    s=sav;
    loc->ndef=max-min;
    loc->bdef=min;
    loc->def=malloc((1+max-min)*sizeof(int));
    for (i=min; i<max; i++) {
	loc->def[i-min]=0;
    }
    while (*s) {
	int from=str_utf8_decomp(&s);
	if (*s) {
	    int val=0, to=str_utf8_decomp(&s);
	    char op=*s;
	    if (op=='+' || op=='-') {
		s++;
		val=*s++-'0'; // FIXME
	    }
	    if (*s==',') s++;
//	    fprintf(stderr, "map: %08x -> %08x\n", from,
//		    (to<<loc->bitshift)+(op=='+'?+1:-1)*val);
	    loc->def[from-min]=(to<<loc->bitshift)+(op=='+'?+1:-1)*val;
	}
    }
    return loc;
}

void locale_done(Locale *loc)
{
    if (loc->def) free(loc->def);
}

int str_cmp_locale(Locale *loc, const char *s1, const char *s2)
{
    int c1, c2, v1=0, v2=0;
    while ((*s1 || *s2) && v1==v2) {
	c1=str_utf8_decomp(&s1);
	c2=str_utf8_decomp(&s2);
	if (c1<loc->bdef || c1>loc->bdef+loc->ndef || !loc->def[c1-loc->bdef])
	    v1=c1<<loc->bitshift;
	else v1=loc->def[c1-loc->bdef];
	if (c2<loc->bdef || c2>loc->bdef+loc->ndef || !loc->def[c2-loc->bdef])
	    v2=c2<<loc->bitshift;
	else v2=loc->def[c2-loc->bdef];
//	v1=loc->def[c1-loc->bdef] ? loc->def[c1-loc->bdef] : c1<<loc->bitshift;
//	v2=loc->def[c2-loc->bdef] ? loc->def[c2-loc->bdef] : c2<<loc->bitshift;
//	fprintf(stderr, "str_locale_cmp: %08x ?= %08x (%08x ?= %08x)\n", c1, c2, v1, v2);
    }
    return v1-v2;
    //return *s1 && *s2 ? v1-v2 : *s1-*s2;
}

int str_ncmp_locale(Locale *loc, const char *s1, const char *s2, int n)
{
    int c1, c2, v1, v2;
    c1=str_utf8_decomp(&s1);
    c2=str_utf8_decomp(&s2);
    if (c1<loc->bdef || c1>loc->bdef+loc->ndef || !loc->def[c1-loc->bdef])
	v1=c1<<loc->bitshift;
    else v1=loc->def[c1-loc->bdef];
    if (c2<loc->bdef || c2>loc->bdef+loc->ndef || !loc->def[c2-loc->bdef])
	v2=c2<<loc->bitshift;
    else v2=loc->def[c2-loc->bdef];
    if (n) n--;
    else return 0;
    while (*s1 && *s2 && v1==v2 && n>0) {
	c1=str_utf8_decomp(&s1);
	c2=str_utf8_decomp(&s2);
	if (c1<loc->bdef || c1>loc->bdef+loc->ndef || !loc->def[c1-loc->bdef])
	    v1=c1<<loc->bitshift;
	else v1=loc->def[c1-loc->bdef];
	if (c2<loc->bdef || c2>loc->bdef+loc->ndef || !loc->def[c2-loc->bdef])
	    v2=c2<<loc->bitshift;
	else v2=loc->def[c2-loc->bdef];
	n--;
//	v1=loc->def[c1-loc->bdef] ? loc->def[c1-loc->bdef] : c1<<loc->bitshift;
//	v2=loc->def[c2-loc->bdef] ? loc->def[c2-loc->bdef] : c2<<loc->bitshift;
//	fprintf(stderr, "str_locale_cmp: %08x ?= %08x (%08x ?= %08x)\n", c1, c2, v1, v2);
    }
    if (n>0 && (!*s1 || !*s2)) return *s1-*s2;
    return v1-v2;
//    return n>0 && *s1 && *s2 ? v1-v2 : *s1-*s2;
}
