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

#include <sscl/network.h>

#include <unistd.h>
#include <netinet/in.h>
#include <linux/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>

//----------------------------------------------------------- class NetConn
NetConn::NetConn(NetServer *serv, const int s, const int len):
	NullStream(s), Stream(s, len, 0)
{
    server=serv;
}

NetConn::NetConn(int family, const char *addr, const int port, const int len):
	NullStream(-1), Stream(-1, len, 0)
{
    if (family==AF_INET) { // Inet socket
	struct sockaddr_in sa;
	struct hostent *hp;
	if ((fd=socket(AF_INET, SOCK_STREAM, 0))<0) {
	    throw Error("E-SYS", errno, sys_errlist[errno], "socket");
	}
	sa.sin_family=AF_INET;
	if (!(hp=gethostbyname(addr))) {
	    close(fd);
	    throw Error("E-SYS", errno, sys_errlist[errno], "gethostbyname");
	}
	bcopy(hp->h_addr, &sa.sin_addr, hp->h_length);
	sa.sin_port=htons(port);
	if (connect(fd, (struct sockaddr *)&sa, sizeof(sa))<0) {
	    close(fd);
	    throw Error("E-SYS", errno, sys_errlist[errno], "connect");
	}
    } else if (family==AF_UNIX) { // Unix socket
	struct sockaddr_un sa;
	if ((fd=socket(AF_UNIX, SOCK_STREAM, 0))<0) {
	    throw Error("E-SYS", errno, sys_errlist[errno], "socket");
	}
	sa.sun_family=AF_UNIX;
	if (addr) {
	    if (strlen(addr)>=UNIX_PATH_MAX)
		throw Error("F-NET", 2, "NetServer::NetServer", "address too long");
	    else strcpy(sa.sun_path, addr);
	} else throw Error("F-NET", 3, "NetServer::NetServer", "address missing");
	if (connect(fd, (struct sockaddr *)&sa, sizeof(sa))<0) {
	    close(fd);
	    throw Error("E-SYS", errno, sys_errlist[errno], "connect");
	}
    } else throw Error("F-NET", 1, "NetServer::NetServer", "unknown type");
    server=NULL;
    fcntl(fd, F_SETFL, O_NONBLOCK);
}

NetConn::~NetConn()
{
    char buf[32];
    shutdown(fd, 2);
    while (read(fd, buf, 30)>0);
    usleep(100);
    close(fd);
    if (server) server->remove(fd);
}

//----------------------------------------------------------- class NetServer
NetServer::NetServer(int family, char *addr, int port, int max_conn, int flags)
{
    if (family==AF_INET) { // Inet socket
	struct sockaddr_in sa;
	sock=socket(PF_INET, SOCK_STREAM, 0);
	if (sock<0) {
	    throw Error("F-SYS", errno, "socket", sys_errlist[errno]);
	    return;
	}
	sa.sin_family=PF_INET;
	if (addr) {
	    inet_aton(addr, &sa.sin_addr);
	} else sa.sin_addr.s_addr=htons(INADDR_ANY);
	sa.sin_port=htons(port);
	if (bind(sock, (struct sockaddr *)&sa, sizeof(sa))<0) {
	    close(sock);
	    throw Error("F-SYS", errno, "bind", sys_errlist[errno]);
	    return;
	}
    } else if (family==AF_UNIX) { // Unix socket
	struct sockaddr_un sa;
	sock=socket(PF_UNIX, SOCK_STREAM, 0);
	if (sock<0) {
	    throw Error("F-SYS", errno, "socket", sys_errlist[errno]);
	    return;
	}
	sa.sun_family=PF_UNIX;
	if (addr) {
	    if (strlen(addr)>=UNIX_PATH_MAX)
		throw Error("F-NET", 2, "NetServer::NetServer", "address too long");
	    else strcpy(sa.sun_path, addr);
	} else throw Error("F-NET", 3, "NetServer::NetServer", "address missing");
	if (bind(sock, (struct sockaddr *)&sa, sizeof(sa))<0) {
	    close(sock);
	    throw Error("F-SYS", errno, "bind", sys_errlist[errno]);
	    return;
	}
    } else throw Error("F-NET", 1, "NetServer::NetServer", "unknown type");
    if (addr) address=strdup(addr);
    listen(sock, max_conn);
    fcntl(sock, F_SETFL, O_NONBLOCK | flags);
    clientnum=0;
}

NetServer::~NetServer()
{
/*    Iterator i;
    for (i=first(); i.get(); i++) {
	delete i.get();
    }
*/
    unlink(address);
    shutdown(sock, 2);
    usleep(100);
    close(sock);
}

/*
char *itoa(int i)
{
    char s[1];
    sprintf(s, "%d", i);
    return strdup(s);
}*/

NetConn *NetServer::accept(int flags)
{
    struct sockaddr sa; unsigned sa_len=16;
    int s=::accept(sock, &sa, &sa_len);
    if (s>0) {
	fcntl(s, F_SETFL, O_NONBLOCK | flags);
	NetConn *nc=new NetConn (this, s);
//	insert(itoa(s), nc);
	clientnum++;
	return nc;
    } else {
	return NULL;
    }
}

void NetServer::remove(int s)
{
//    this->erase(s);
    clientnum--;
}
