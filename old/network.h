#ifndef __NETWORK_H
#define __NETWORK_H

/**
 * This header contains the network code used for both the server and client.
 *
 * server_* are server-specific.
 * client_* are client-specific.
 *
 * All others are generic and/or used for both.
 **/
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "debug.h"

/**
 * in_addr()
 * isa: struct sockaddr pointer for server sockaddr     [in]
 *
 * Fetches the IPv4 or IPv6 address, depending on which is supported.
 **/
void *in_addr(struct sockaddr *isa){
        if(isa->sa_family == AF_INET)
                return &(((struct sockaddr_in*)isa)->sin_addr);

        return &(((struct sockaddr_in6*)isa)->sin6_addr);
}

/**
 * sendall()
 * s:		Socket [file descriptor] to send data to	[in]
 * buffer:	The data to send to the socket			[in]
 *
 * Recursive send() for socket, since TCP/IP can't guarantee all of buffer will be sent in one go.
 *
 * Returns 0 on failure, otherwise total bytes sent.
 **/
int sendall(int s, char *buffer){
	int pos = 0;
	int len = strlen(buffer);
	int left = len;
	int curr = 0;

	while((curr = send(s, buffer+pos, left, 0)) != -1){
		pos += curr;
		left -= curr;
	}

	memset(buffer, '\0', strlen(buffer));

	return pos;
}

int recvbufflen(int s){
	char *tmp = (char*)malloc(sizeof(char) * 5);
	memset(tmp, '\0', sizeof(tmp));

	recv(s, tmp, 5, 0);

	int len = atoi(tmp);

	free(tmp);

	return len;
}

void sendbufflen(int s, int len){
	char *tmp = (char*)malloc(sizeof(char) * 5);
	memset(tmp, '\0', sizeof(tmp));

	sprintf(tmp, "%05d", len);

	if(!sendall(s, tmp))
		D(("error sendbufflen() -> sendall()"));

	free(tmp);
}

/**
 * recvall()
 *
 * Same as sendall().  Done a little differently though.
 **/
int recvall(int s, char *buffer, int len){
D(("Receiving %d bytes from %d", len, s));
	int curr = 0;
	int pos = 0;
	int left = len;

	/**
	 * If we weren't using blocking I/O for sockets, could do "!= 0" instead...
	 * However, "-1" is the only check possible right now...
	 **/
	while(pos < len){
D(("while(%d < %d)", pos, len));
		curr = recv(s, buffer+pos, left, 0);

		if(curr == -1){
			perror("recvall()");
			pos = 0;
			break;
		}

		pos += curr;
		left -= curr;
D(("Read %d bytes (%d / %d total)", curr, pos, len));
	}

	return pos;
}

/*
#define BACKLOG 10

// File descriptors needed
int server_fd, connfd, client_fd;

struct addrinfo hints, *server_info, *p;

// Connector information
struct sockaddr_storage server_clientaddr;

socklen_t sin_size;

void *get_in_addr(struct sockaddr *sa){
	// If using IPv4 (AF_INET), return IPv4 struct, otherwise 6
	if(sa->sa_family == AF_INET)
		return &(((struct sockaddr_in*)sa)->sin_addr);

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int server_init(struct addrinfo hints, char *host){
	int rv = 0;

	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if((rv = getaddrinfo(host, PORT, &hints, &server_info)) != 0){
		D(("getaddrinfo(%d): %s", rv, gai_strerror(rv)));

		return 0;
	}

	for(p = server_info; p != NULL; p = p->ai_next){
		if((server_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
			perror("server: socket");
			continue;
		}

		if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &rv, sizeof(int)) == -1){
			perror("setsockopt");
			return 0;
		}

		if(bind(server_fd, p->ai_addr, p->ai_addrlen) == -1){
			close(server_fd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if(!p){
		D(("Unable to bind to %s:%s", host, PORT));
		return 0;
	}

	freeaddrinfo(server_info);

D(("Bound to %s on %s", host, PORT));
	return 1;
}

void server_listen(){
	D(("server_fd = %d", server_fd));
	while(1){
		sin_size = sizeof(server_clientaddr);

		connfd = accept(server_fd, (struct sockaddr*)&server_clientaddr, &sin_size);

		if(connfd == -1){
//			D(("connfd = %d"));
			continue;
		} else
			D(("Client connecting."));

		close(connfd);
	}
}
*/

#endif
