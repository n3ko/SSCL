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

#include <sscl/error.h>
#include <sscl/stream.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int sscl_buf_len=SSCL_BUF_LEN;

NullStream::NullStream(const int f)
{
    fd=f;
}

NullStream::NullStream(const char *file, const int flags)
{
    fd=open(file, flags);
}

int NullStream::get_fd()
{
    return fd;
}

InStream::InStream(const int f, const int len): NullStream(f)
{
    ibufl=len; icnt=0;
    ibuf=ibegin=inl=new char[len];
}

InStream::InStream(const char *file, const int len): NullStream(file, O_RDONLY)
{
    ibufl=len; icnt=0;
    ibuf=ibegin=inl=new char[len];
}

InStream::~InStream()
{
    delete ibuf;
}

/**/
int InStream::try_read()
{
    // Read from the file if data is available
    register int r;
    bool eof=false;
    do {
	if (icnt==ibufl) return 0; // Buffer is full, read is impossible
	r=0;
	if (ibegin+icnt<ibuf+ibufl) {
	    // No buffer overflow
	    if (ibegin+icnt<ibuf+ibufl) {
		r=::read(fd, ibegin+icnt, ibuf+ibufl-ibegin-icnt);
		if (!r) eof=true;
	    }
	} else {
	    r=::read(fd, ibegin+icnt-ibufl, ibufl-icnt);
	    if (!r) eof=true;
	}
	if (r>0) icnt+=r;
	else if (r<0 && errno!=EAGAIN) throw Error("F-SYS", errno, "read", sys_errlist[errno]);
    } while (r>0);
    return eof ? -1 : 0;
}

int InStream::get_c()
{
    bool eof=false;
    int ret;
    if (!icnt) eof=try_read();
    if (icnt--) {
	ret=*ibegin++;
	if (inl==ibegin-1) inl++;
	if (ibegin==ibuf+ibufl) ibegin=ibuf;
	return ret;
    }
    if (eof) return EOF;
    else return 0;
}

int InStream::get_c_wait()
{
    int c;
    while (!(c=get_c())) usleep(100);
    return c;
}

#define ENDL(x) ((x)=='\n')
#define ABSV(x) ((x)<ibegin ? (x)+ibufl : (x))
int InStream::get_s(char *buffer, int n)
{
    // Read from the file if data is available
    bool eof=(try_read()<0);
    // Search for a newline character
    while (inl<ibuf+ibufl && inl<ibegin+icnt && !ENDL(*inl)) inl++;
    if (inl==ibuf+ibufl) inl=ibuf;
    char *iend=ibegin+icnt;
    if (iend>ibuf+ibufl) iend-=ibufl;
    while (inl<iend && !ENDL(*inl)) inl++;
    // If a newline was found then copy the line to the buffer
    if (ENDL(*inl)) {
	char *d=buffer, *s=ibegin;
	if (inl<s) {
	    while (s<ibuf+ibufl && n) if (*s!='\r') *d++=*s++, n--; else s++;
	    s=ibuf;
	}
	while (s<inl && n) if (*s!='\r') *d++=*s++, n--; else s++;
	*d=0; inl++;
	icnt-=ABSV(inl)-ibegin;
	ibegin=inl;
	return d-buffer;
    } else {
	if (eof) return -1;
	else return 0;
    }
}

int InStream::get_s_wait(char *buffer, int n)
{
    int s;
    while (!(s=get_s(buffer, n))) usleep(100);
    return s;
}

OutStream::OutStream(const int f, const int len): NullStream(f)
{
}

OutStream::~OutStream()
{
}

int OutStream::put_c(const char c)
{
    return ::write(fd, &c, 1);
}

int OutStream::write(const char *buffer, int n)
{
    int s=n, w;
    while (s>0) {
	s-=w=::write(fd, buffer+n-s, s);
	if (!w) usleep(100);
    }
    return w<0 ? w : n;
}

int OutStream::write(const char *buffer)
{
    return write(buffer, strlen(buffer));
}

Stream::Stream(int f, const int ilen, const int olen):
	NullStream(f), InStream(f, ilen), OutStream(f, olen)
{
}
