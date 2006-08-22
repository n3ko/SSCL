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

#include <errno.h>

#include <sscl/ssclc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define IDX(s, x) (((x)<(s)->ibegin ? (x)+(s)->ibufl : (x))-(s)->ibegin)
#define INC(s, p) (((p)<(s)->ibuf+(s)->ibufl-1 ? (p)++ : ((p)=(s)->ibuf)))

Stream *stream_init_fd(Stream *s, int fd, const int ilen)
{
    s->fd=fd;
    s->ibufl=ilen; s->icnt=0;
    s->ibuf=s->ibegin=s->inl=ilen ? malloc(ilen+1) : NULL;
    return s;
}

Stream *stream_new_fd(int fd, const int ilen)
{
    Stream *s=NEW(Stream);
    return stream_init_fd(s, fd, ilen);
}

Stream *stream_init_file(Stream *s, const char *fn, int flags, const int ilen)
{
    s->fd=open(fn, flags);
    s->ibufl=ilen; s->icnt=0;
    s->ibuf=s->ibegin=s->inl=malloc(ilen+1);
    return s;
}

Stream *stream_new_file(const char *fn, int flags, const int ilen)
{
    Stream *s=NEW(Stream);
    return stream_init_file(s, fn, flags, ilen);
}

void stream_done(Stream *s)
{
    if (s->fd>=0) close(s->fd);
    free(s->ibuf);
}

void stream_free(Stream *s)
{
    stream_done(s);
    free(s);
}

/* Reads in as much data as possible to the circular buffer
 * Returns the number of bytes read, 0 if no data available,
 *         -1: EOF, -2: Error
 */
int stream_try_read(Stream *s)
{
    // Read from the file if data is available
    register int r, rd=0;
    Bool eof=false;
    do {
	r=0;
	if (s->icnt==s->ibufl) break; // Buffer is full, read is impossible
	if (s->ibegin+s->icnt<s->ibuf+s->ibufl) {
	    // No buffer overflow
	    r=read(s->fd, s->ibegin+s->icnt, s->ibuf+s->ibufl-s->ibegin-s->icnt);
	    if (!r) eof=true;
	} else {
	    // Overflow
	    r=read(s->fd, s->ibegin+s->icnt-s->ibufl, s->ibufl-s->icnt);
	    if (!r) eof=true;
	}
	if (r>0) { s->icnt+=r; rd+=r; }
	else if (r<0) r=errno==EAGAIN ? 0 : -2;
	else if (!r) r=-1;
    } while (r>0);
    return r<0 ? r : rd;
}

int stream_read(Stream *s, char *buffer, int n)
{
    char *d=buffer;
    int rd=0, ret;
    if (s->icnt<n) rd=stream_try_read(s);
    n=ret=n<s->icnt ? n : s->icnt;
    while (n) *d++=*s->ibegin, n--, INC(s, s->ibegin), s->icnt--;
    *d=0;
fprintf(stderr, "stream_read: ret:%d rd:%d icnt:%d\n", ret, rd, s->icnt);
    return ret>0 ? ret : rd;
}

int stream_get_c(Stream *s)
{
    Bool eof=false;
    int ret;
    if (!s->icnt) eof=stream_try_read(s)<0;
    if (s->icnt--) {
	ret=*s->ibegin++;
	if (s->inl==s->ibegin-1) s->inl++;
	if (s->ibegin==s->ibuf+s->ibufl) s->ibegin=s->ibuf;
	return ret;
    }
    if (eof) return EOS;
    else return 0;
}

int stream_get_c_wait(Stream *s)
{
    int c;
    while (!(c=stream_get_c(s))) usleep(100);
    return c;
}

#define ENDL(x) ((x)=='\n')
int stream_get_s(Stream *s, char *buffer, int n)
{
    int rd=0;
    // Read from the file if data is available
//    int rd=stream_try_read(s);
    // Search for a newline character
    int nb=s->icnt-IDX(s, s->inl);
//fprintf(stderr, "\n\nBEG:get_s (%s): nl:%d icnt:%d ibuf:%p ibegin:%p inl=%p\n", s->ibuf, IDX(s, s->inl), s->icnt, s->ibuf, s->ibegin, s->inl);
    while (nb>0 && !ENDL(*s->inl)) INC(s, s->inl), nb--;
    // Found?
    if (!nb) {
	// NL not found, trying to read
	rd=stream_try_read(s);
	// Search further
	nb=s->icnt-IDX(s, s->inl);
	while (nb>0 && !ENDL(*s->inl)) INC(s, s->inl), nb--;
    }
    // If a newline was found then copy the line to the buffer
    if ((s->ibufl==s->icnt) || IDX(s, s->inl)<s->icnt) {
	char *d=buffer;
	while (!ENDL(*s->ibegin) && s->icnt>0 && n) {
	    if (*s->ibegin!='\r') *d++=*s->ibegin, n--;
	    INC(s, s->ibegin);
	    s->icnt--;
	}
	*d=0; if (ENDL(*s->inl)) INC(s, s->inl);
	if (ENDL(*s->ibegin)) { INC(s, s->ibegin); s->icnt--; }
//fprintf(stderr, "END:get_s (%s): nl:%d icnt:%d ibuf:%p ibegin:%p inl=%p\n", s->ibuf, IDX(s, s->inl), s->icnt, s->ibuf, s->ibegin, s->inl);
	return d-buffer;
    } else {
	if (rd<0) return rd;
	else { *buffer=0; return 0; }
    }
}
/*
int stream_get_s(Stream *s, char *buffer, int n)
{
    char *iend;
    // Read from the file if data is available
    Bool eof=(stream_try_read(s)<0);
    // Search for a newline character
    while (s->inl<s->ibuf+s->ibufl && s->inl<s->ibegin+s->icnt && !ENDL(*s->inl)) s->inl++;
    if (s->inl==s->ibuf+s->ibufl) s->inl=s->ibuf;
    iend=s->ibegin+s->icnt;
    if (iend>=s->ibuf+s->ibufl) iend-=s->ibufl;
    while (s->inl<iend && !ENDL(*s->inl)) s->inl++;
fprintf(stderr, "get_s NL(): %d %d %p %p %p %p\n", s->inl-s->ibuf, s->icnt, s->ibuf, s->ibegin, iend, s->inl);
    // If a newline was found then copy the line to the buffer
//    if (ENDL(*s->inl)) {
    if ((s->ibegin>=iend && s->ibufl && (s->inl<iend || s->inl>=s->ibegin))
	    || (s->ibufl==s->icnt)
	    || (s->ibegin<iend && s->ibegin<=s->inl && s->inl<iend)) {
	char *d=buffer, *st=s->ibegin;
	if (s->inl<st) {
	    while (st<s->ibuf+s->ibufl && n) if (*st!='\r') *d++=*st++, n--; else st++;
	    st=s->ibuf;
	}
	while (st<s->inl && n) if (*st!='\r') *d++=*st++, n--; else st++;
	*d=0; s->inl++;
fprintf(stderr, "get_s IDX: %d\n", IDX(s, s->inl));
	s->icnt-=IDX(s, s->inl);
	s->ibegin=s->inl;
	return d-buffer;
    } else {
	if (eof) return -1;
	else return 0;
    }
}
*/
int stream_get_s_wait(Stream *s, char *buffer, int n)
{
    int ret;
    while (!(ret=stream_get_s(s, buffer, n))) {
	usleep(100);
    }
    return ret;
}

int stream_put_c(Stream *s, const char c)
{
    return write(s->fd, &c, 1);
}

int stream_write(Stream *s, const char *buffer, int n)
{
    int st=n, w;
    while (st>0) {
	st-=w=write(s->fd, buffer+n-st, st);
	if (!w) usleep(100);
    }
    return w<0 ? w : n;
}

int stream_put_s(Stream *s, const char *buffer)
{
    return stream_write(s, buffer, str_len(buffer));
}

int stream_print_v(Stream *s, const char *format, va_list args)
{
    char buf[8001];
    str_printv(buf, format, 8000, args);
    return stream_put_s(s, buf);
}

int stream_print(Stream *s, const char *format,...)
{
    int ret;
    char buf[8001];
    va_list args;
    va_start(args, format);
    ret=stream_put_s(s, str_printv(buf, format, 8000, args));
    va_end(args);
    return ret;
}
