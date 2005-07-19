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
#include <sscl/ssclc.h>

namespace SSCL {

class NullStream {
    public:
	NullStream(const int fd);
	NullStream(const char *file, const int flags);
//	NullStream(const int fd) {stream_init_fd(&cs, fd, 0);}
//	NullStream(const char *file, const int flags) {stream_init_file(&cs, file, flags, 0);}
	~NullStream() {stream_done(&cs);}
	int get_fd() {return cs.fd;}
    protected:
	Stream cs;
};

class InStream: virtual public NullStream {
    public:
	InStream(const int fd, const int len=SSCL_BUF_LEN);
	InStream(const char *file, const int flags, const int len=SSCL_BUF_LEN);
//	InStream(const int fd, const int len=SSCL_BUF_LEN): NullStream(fd)
//		{stream_init_fd(&cs, fd, len);}
//	InStream(const char *file, const int flags, const int len=SSCL_BUF_LEN): NullStream(file, flags)
//		{stream_init_file(&cs, file, flags, len);}
//	virtual ~InStream() {stream_done(&cs);}
	virtual ~InStream() {}
	virtual int get_c() {return stream_get_c(&cs);}
	virtual int get_c_wait() {return stream_get_c_wait(&cs);}
	int get_s(char *buffer, int n) {return stream_get_s(&cs, buffer, n);}
	int get_s_wait(char *buffer, int n) {return stream_get_s_wait(&cs, buffer, n);}
};

class OutStream: virtual public NullStream {
    public:
	OutStream(int fd, const int len=SSCL_BUF_LEN);
//	OutStream(int fd, const int len=SSCL_BUF_LEN): NullStream(fd)
//		{stream_init_fd(&cs, fd, len);}
	virtual ~OutStream() {}
	virtual int put_c(const char c) {return stream_put_c(&cs, c);}
	virtual int write(const char *buffer, int n) {return stream_write(&cs, buffer, n);}
	virtual int put_s(const char *buffer) {return stream_put_s(&cs, buffer);}
	virtual int print_v(const char *format, va_list args) {
	    return stream_print_v(&cs, format, args);
	}
	virtual int print(const char *format,...) {
	    int ret;
	    va_list args;
	    va_start(args, format);
	    ret=stream_print_v(&cs, format, args);
	    va_end(args);
	    return ret;
	}
};

class Stream: public InStream, public OutStream {
    public:
	Stream(int fd, const int ilen=SSCL_BUF_LEN, const int olen=SSCL_BUF_LEN);
//	Stream(int fd, const int ilen=SSCL_BUF_LEN, const int olen=SSCL_BUF_LEN):
//		NullStream(fd), InStream(fd, ilen), OutStream(fd) {}
};

} /* namespace SSCL */

#endif /* _SSCL_STREAM_H */
