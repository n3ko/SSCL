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
#ifndef _SSCL_STREAM_H
#define _SSCL_STREAM_H

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

//#include <sscl/base.h>
#include <sscl/config.h>
#include <sscl/object.h>

class NullStream {
    public:
	NullStream(const int f);
	NullStream(const char *file, const int flags);
	//NullStream(char *file);
	int get_fd();
    protected:
	int fd;
};

class InStream: virtual public NullStream {
    public:
	InStream(const int f, const int len=SSCL_BUF_LEN);
	InStream(const char *file, const int len=SSCL_BUF_LEN);
	virtual ~InStream();
	virtual int get_c();
	int get_s(char *buffer, int n);
    protected:
	int try_read();
	// Representation:
	//	[X][X][X][ ][ ][ ][X][X][X][X]
	//	          ^iend    ^ibegin
	int ibufl;
	char *ibuf;
	char *ibegin;
//	char *iend;
	int icnt;
	char *inl;
};

class OutStream: virtual public NullStream {
    public:
	OutStream(int f, const int len=SSCL_BUF_LEN);
	virtual ~OutStream();
	virtual int put_c(const char c);
	virtual int write(const char *buffer, int n);
	virtual int write(const char *buffer);
    protected:
};

class Stream: public InStream, public OutStream {
    public:
	Stream(int f, const int ilen=SSCL_BUF_LEN, const int olen=SSCL_BUF_LEN);
    protected:
};

/*
class Stream: public InStream, public OutStream {
    public:
	Stream(int f, const int len);
	virtual ~Stream() {delete buf;};
	virtual int get_c();
	int get_s(char *buffer, int n);
	virtual int put_c(const char c) {return ::write(fd, &c, 1);};
	virtual int write(const char *buffer, int n) {return ::write(fd, buffer, n);};
	virtual int write(const char *buffer) {return ::write(fd, buffer, strlen(buffer));};
	int get_fd() {return fd;};
    protected:
	int fd;
	int buflen;
	char *buf;
	char *end;
	char *p;
};
*/

#endif /* _SSCL_STREAM_H */
