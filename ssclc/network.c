/* ISDK - Information System Development Kit
 * Copyright (C) 1998-1999 Szilard Hajba, Peter Vazsonyi
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
#include <sscl/ssclc.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <netdb.h>
#include <errno.h>

///////////////
//  NetConn  //
///////////////
NetConn *netconn_init(NetConn *net, const NetConnFamily family, const char *addr,
	const int port, const int buflen)
{
    int fd;
    if (family==nf_inet) { // Inet socket
	struct sockaddr_in sa;
	struct hostent *hp;
	if ((fd=socket(PF_INET, SOCK_STREAM, 0))<0) return NULL;
	sa.sin_family=AF_INET;
	if (!(hp=gethostbyname(addr))) { close(fd); return NULL; }
	bcopy(hp->h_addr, &sa.sin_addr, hp->h_length);
	sa.sin_port=htons(port);
	if (connect(fd, (struct sockaddr*)&sa, sizeof(sa))<0) {
	    close(fd); return NULL;
	}
    } else if (family==nf_unix) { // Unix socket
	struct sockaddr_un sa;
	if ((fd=socket(PF_UNIX, SOCK_STREAM, 0))<0) return NULL;
	sa.sun_family=AF_UNIX;
	if (addr) {
	    char *p, *daddr=str_dup(addr);
	    int cstat=-1;
	    p=str_split(daddr, ":");
	    while (p && cstat) {
		if (str_len(p)>=sizeof(sa.sun_path)-1) return NULL;
		else str_cpy(sa.sun_path, p, sizeof(sa.sun_path)-1);
		cstat=connect(fd, (struct sockaddr *)&sa, sizeof(sa));
		p=str_split(NULL, ":");
	    }
	    free(daddr);
	    if (cstat<0) {
		close(fd);
		return NULL;
	    }
	} else return NULL;
    } else return NULL;
    stream_init_fd(STREAM(net), fd, buflen);
    fcntl(net->_parent.fd, F_SETFL, O_NONBLOCK);
    net->server=NULL;
    return net;
}

NetConn *netconn_init_server(NetConn *net, NetServer *server, const int fd, const int buflen)
{
    stream_init_fd(STREAM(net), fd, buflen);
    net->server=server;
    return net;
}

NetConn *netconn_new(const NetConnFamily family, const char *addr, const int port, const int buflen)
{
    NetConn *net=NEW(NetConn);
    return netconn_init(net, family, addr, port, buflen);
}

void netconn_done(NetConn *net)
{
    char buf[32];
    if (net->_parent.fd>=0) {
	shutdown(net->_parent.fd, 2);
	while (read(net->_parent.fd, buf, 30)>0);
	usleep(100);
	stream_done(STREAM(net));
	if (net->server) netserver_remove(net->server, net->_parent.fd);
    }
}

void netconn_free(NetConn *net)
{
    netconn_done(net);
    free(net);
}

char *netconn_get_ip(NetConn *net)
{
    struct sockaddr sa;
    socklen_t sal=sizeof(sa);
    if (getpeername(net->_parent.fd, &sa, &sal)) {
	return NULL;
    } else {
	if (sa.sa_family==PF_INET)
	    return inet_ntoa(((struct sockaddr_in*)&sa)->sin_addr);
	else return NULL;
    }
}

///////////////
// NetServer //
///////////////
NetServer *netserver_init(NetServer *serv, const NetConnFamily family, const char *addr,
	const int port, const int max_conn, const int flags)
{
    serv->family=family;
    if (family==nf_inet) { // Inet socket
	struct sockaddr_in sa;
	int so_val=1;
	serv->fd=socket(PF_INET, SOCK_STREAM, 0);
	if (serv->fd<0) return NULL;
	setsockopt(serv->fd, SOL_SOCKET, SO_REUSEADDR,
		&so_val, sizeof(so_val));
	sa.sin_family=AF_INET;
	if (addr) {
	    inet_aton(addr, &sa.sin_addr);
	} else sa.sin_addr.s_addr=htons(INADDR_ANY);
	sa.sin_port=htons(port);
	if (bind(serv->fd, (struct sockaddr *)&sa, sizeof(sa))<0) {
	    close(serv->fd); return NULL;
	}
    } else if (family==nf_unix) { // Unix socket
	struct sockaddr_un sa;
	serv->fd=socket(PF_UNIX, SOCK_STREAM, 0);
	if (serv->fd<0) return NULL;
	sa.sun_family=PF_UNIX;
	if (addr) {
	    if (str_len(addr)>=sizeof(sa.sun_path)-1) return NULL;
	    else str_cpy(sa.sun_path, addr, sizeof(sa.sun_path)-1);
	} else return NULL;
	if (bind(serv->fd, (struct sockaddr *)&sa, sizeof(sa))<0) {
	    close(serv->fd);
	    return NULL;
	}
    } else return NULL;
    if (addr) serv->address=strdup(addr);
    listen(serv->fd, max_conn);
    fcntl(serv->fd, F_SETFL, O_NONBLOCK | flags);
    serv->clientnum=0;
    return serv;
}

NetServer *netserver_new(const NetConnFamily family, const char *addr, const int port,
	const int max_conn, const int flags)
{
    NetServer *serv=NEW(NetServer);
    return netserver_init(serv, family, addr, port, max_conn, flags);
}

void netserver_done(NetServer *serv)
{
/*    Iterator i;
    for (i=first(); i.get(); i++) {
	delete i.get();
    }
*/
    char buf[32];
    shutdown(serv->fd, 2);
    while (read(serv->fd, buf, 30)>0);
    usleep(100);
    if (serv->family==nf_unix) unlink(serv->address);
    close(serv->fd);
}

NetConn *netserver_accept(NetServer *serv, NetConn *net, const int flags, const int buflen)
{
    struct sockaddr sa; unsigned sa_len=16;
    int s=accept(serv->fd, &sa, &sa_len);
    if (s>0) {
	fcntl(s, F_SETFL, O_NONBLOCK | flags);
	netconn_init_server(net, serv, s, buflen);
//	insert(itoa(s), nc);
	serv->clientnum++;
	return net;
    } else return NULL;
}

void netserver_remove(NetServer *serv, int s)
{
//    this->erase(s);
    serv->clientnum--;
}
