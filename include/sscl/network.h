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
#ifndef _SSCL_NETWORK_H
#define _SSCL_NETWORK_H

#include <sscl/avltree.h>
#include <sscl/stream.h>
#include <sscl/error.h>
#include <sys/socket.h>

namespace SSCL {

class NetConn;
class NetServer;

class NetConn {
    public:
	NetConn() {netconn_init_server(&cs, NULL, -1, 0);}
	NetConn(const NetConnFamily family, const char *addr, const int port, const int buflen=SSCL_BUF_LEN)
	{
	    errno=0;
	    if (!netconn_init(&cs, family, addr, port, buflen)) {
		throw Error("E-SYS", errno, errno ? strerror(errno) : "Unknown error",
			"NetServer::NetServer");
	    }
	}
	char *get_ip() {return netconn_get_ip(&cs);}
	virtual ~NetConn() {/*if (cs._parent.fd>0)*/ netconn_done(&cs);}
	virtual int get_c() {return stream_get_c(&cs._parent);}
	virtual int get_c_wait() {return stream_get_c_wait(&cs._parent);}
	virtual int get_c_wait_t(int msec) {return stream_get_c_wait_t(&cs._parent, msec);}
	int get_s(char *buffer, int n) {return stream_get_s(&cs._parent, buffer, n);}
	int get_s_wait(char *buffer, int n) {return stream_get_s_wait(&cs._parent, buffer, n);}
	int get_s_wait_t(char *buffer, int n, int msec) {return stream_get_s_wait_t(&cs._parent, buffer, n, msec);}
	virtual int put_c(const char c) {return stream_put_c(&cs._parent, c);}
	virtual int write(const char *buffer, int n) {return stream_write(&cs._parent, buffer, n);}
	virtual int put_s(const char *buffer) {return stream_put_s(&cs._parent, buffer);}
	virtual int print_v(const char *format, va_list args) {
	    return stream_print_v(&cs._parent, format, args);
	}
	virtual int print(const char *format,...) {
	    int ret;
	    va_list args;
	    va_start(args, format);
	    ret=stream_print_v(&cs._parent, format, args);
	    va_end(args);
	    return ret;
	}
    private:
	friend class NetServer;
	::NetConn cs;
};

class NetServer/*: public AVLTree*/ {
    public:
	NetServer(const NetConnFamily family, const char *addr, const int port,
		const int max_conn, const int flags=0)
	{
	    errno=0;
	    if (!netserver_init(&cs, family, addr, port, max_conn, flags)) {
		throw Error("E-SYS", errno, errno ? strerror(errno) : "Unknown error",
			"NetServer::NetServer");
	    }
	}
	~NetServer() {netserver_done(&cs);}
	NetConn *accept(int flags=0, int buflen=SSCL_BUF_LEN) {
	    NetConn *nc=new NetConn;
	    ::NetConn *r=netserver_accept(&cs, &nc->cs, flags, buflen);
	    if (r) return nc;
	    delete nc; return NULL;
	}
	//void NetServer::add(int s, NetConn *cl);
	void remove(int s) {netserver_remove(&cs, s);}
    private:
	::NetServer cs;
};

} /* namespace SSCL */

#endif /* _SSCL_NETWORK_H */
