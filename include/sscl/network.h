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

//================================================================ Exceptions
#define ERR_NET_BIND "F-SYS-0001"

class NetConn;
class NetServer/*: public AVLTree*/ {
    public:
	NetServer(int family, char *addr, int port, int max_conn, int flags=0);
	~NetServer();
	NetConn *accept(int flags=0);
	void NetServer::add(int s, NetConn *cl);
	void remove(int s);
    private:
	int sock;
	int clientnum;
	char *address;
//	map<int, NetConn*> client;
};

class NetConn: public Stream {
    public:
	NetConn(NetServer *serv, const int s, const int len=SSCL_BUF_LEN);
	NetConn(int family, const char *addr, const int port, const int len=SSCL_BUF_LEN);
	~NetConn();
//	void send(string text);
//	char *recv();
    private:
	NetServer *server;
};

#endif /* _SSCL_NETWORK_H */
