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
    close(s->fd);
    free(s->ibuf);
}

void stream_free(Stream *s)
{
    stream_done(s);
    free(s);
}

int stream_try_read(Stream *s)
{
    // Read from the file if data is available
    register int r;
    Bool eof=false;
    do {
	if (s->icnt==s->ibufl) return 0; // Buffer is full, read is impossible
	r=0;
	if (s->ibegin+s->icnt<s->ibuf+s->ibufl) {
	    // No buffer overflow
	    if (s->ibegin+s->icnt<s->ibuf+s->ibufl) {
		r=read(s->fd, s->ibegin+s->icnt, s->ibuf+s->ibufl-s->ibegin-s->icnt);
		if (!r) eof=true;
	    }
	} else {
	    r=read(s->fd, s->ibegin+s->icnt-s->ibufl, s->ibufl-s->icnt);
	    if (!r) eof=true;
	}
	if (r>0) s->icnt+=r;
	else if (r<0 && errno!=EAGAIN) return -2;
    } while (r>0);
    return eof ? -1 : 0;
}

int stream_get_c(Stream *s)
{
    Bool eof=false;
    int ret;
    if (!s->icnt) eof=stream_try_read(s);
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
#define ABSV(x) ((x)<s->ibegin ? (x)+s->ibufl : (x))
int stream_get_s(Stream *s, char *buffer, int n)
{
    char *iend;
    // Read from the file if data is available
    Bool eof=(stream_try_read(s)<0);
    // Search for a newline character
    while (s->inl<s->ibuf+s->ibufl && s->inl<s->ibegin+s->icnt && !ENDL(*s->inl)) s->inl++;
    if (s->inl==s->ibuf+s->ibufl) s->inl=s->ibuf;
    iend=s->ibegin+s->icnt;
    if (iend>s->ibuf+s->ibufl) iend-=s->ibufl;
    while (s->inl<iend && !ENDL(*s->inl)) s->inl++;
    // If a newline was found then copy the line to the buffer
    if (ENDL(*s->inl)) {
	char *d=buffer, *st=s->ibegin;
	if (s->inl<st) {
	    while (st<s->ibuf+s->ibufl && n) if (*st!='\r') *d++=*st++, n--; else st++;
	    st=s->ibuf;
	}
	while (st<s->inl && n) if (*st!='\r') *d++=*st++, n--; else st++;
	*d=0; s->inl++;
	s->icnt-=ABSV(s->inl)-s->ibegin;
	s->ibegin=s->inl;
	return d-buffer;
    } else {
	if (eof) return -1;
	else return 0;
    }
}

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
