/****************************
 * Network.h
 *
 * Used for handling network-side code.
 * This is a pseudo-C class for networking stuff.  Reworked to make things easier.
 ****************************/
#ifndef __NETWORK_H
#define __NETWORK_H

#include "global.h"

// Used for sockets
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/**
 * NetSockErr()
 * err:	Error number to print message about	[in]
 *
 * Displays error message.
 **/
void NetSockErr(int err){
	D(("Socket error %d: %s", err, strerror(err)));
}

/**
 * NetSockErrOk()
 * err:	Error number of socket call	[in]
 *
 * Since using blocking sockets, there can be socket errors that are non-fatal.
 * This checks err against this, returns 1 if the error is non-fatal, 0 if it is fatal.
 **/
int NetSockErrOk(int err){
	// Known error codes to not be fatal to a socket stream
	if((err != EINTR) && (err != EAGAIN) && (err != EWOULDBLOCK)){
		NetSockErr(err);

		return 0;
	}

	D(("Encountered non-fatal socket error...continuing with transmission."));

	return 1;
}

/**
 * NetSetOpt()
 * sock:	Socket to set the option for	[in]
 * opt:		Option to set on socket		[in]
 *
 * Returns returned value of setsockopt()
 **/
int NetSetOpt(int sock, int opt){
	int true = 1;

	return setsockopt(sock, SOL_SOCKET, opt, &true, sizeof(int));
}

/**
 * NetServerCreate()
 * port:	The port number to create the server on	[in]
 *
 * Returns 0 on failure, socket FD on success.
 **/
int NetServerCreate(int port){
	int sock = -1;
	int enable = 1;

	struct sockaddr_in net;

	if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
		perror("socket()");
		return 0;
	}

	D(("Created socket %d", sock));

	NetSetOpt(sock, SO_REUSEADDR);

	mem0(&net);

	net.sin_family = AF_INET;
	net.sin_port = htons(port);
	net.sin_addr.s_addr = INADDR_ANY;

	if(bind(sock, (struct sockaddr*)&net, sizeof(net)) == -1){
		perror("bind()");
		close(sock);

		return 0;
	}

//	if(listen(sock, SO_MAXCONN) == -1){
	if(listen(sock, 10) == -1){
		perror("listen()");
		close(sock);

		return 0;
	}

	return sock;
}
/**
 * NetClientCreate()
 * ip:		IP address to connect to	[in]
 * port:	Port number to connect to	[in]
 *
 * Returns 0 on failure, socket FD on success.
 **/
int NetClientCreate(char *ip, int port){
	struct sockaddr_in net;

	int sock = 0;

	if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
		perror("socket()");
		return 0;
	}

	mem0(&net);

	net.sin_family = AF_INET;
	net.sin_port = htons(port);

	if(inet_pton(AF_INET, ip, &net.sin_addr) != -1){
		perror("inet_pton()");
		close(sock);

		return 0;
	}

	if(connect(sock, (struct sockaddr*)&net, sizeof(net)) == -1){
		perror("connect()");
		close(sock);

		return 0;
	}

	return sock;
}

/**
 * sendall()
 * sock:	Socket FD to send data to	[in]
 * data:	Buffer of data to send to sock	[in]
 *
 * Sends all of the data to sock, returns 0 on failure, 1 on success.
 **/
int sendall(int sock, char *data){
	// Get length of data, as well as init other variables
	int len = strlen(data);
	int pos = 0;
	int curr = 0;
	int left = len;
	int err = 0;

	// While current buffer position != data length
	while(pos <= len){
		// Send some data to the socket
		curr = send(sock, data+pos, left, 0);

		// Check if an error occured (-1)
		if(curr != -1){
			// Update position of buffer by how much was actually sent
			pos += curr;

			// We have so much left to send
			left -= curr;
		} else{
			// Holder for error number (safe-guard)
			err = errno;

			// If we ran into a fatal error, cancel the sending
			if(!NetSockErrOk(err))
				return 0;
		}
	}

	// Empty the data buffer
	mem0(data);

	// Since we sent all the data, just make error checking on function easier
	return 1;
}

/**
 * sendlenall()
 *
 * Same as sendall(), but sends the length of data first, then the actual data.
 **/
int sendlenall(int sock, char *data){
	// Allocate temporary buffer to send length (5-character buffer)
	char *tmp = mem(5);
	mem0(tmp);

	// Pad the length with 0's if it's less than 5 digits long
	sprintf(tmp, "%05d", strlen(data));

	if(!sendall(sock, tmp)){
		free(tmp);

		return 0;
	}

	// May not be needed, but used to cause a breather in transmission
	sleep(1);

	if(!sendall(sock, data))
		return 0;

	free(tmp);

	return 1;
}

/**
 * recvall()
 * socket:	Socket to receive data from	[in]
 * buff:	Buffer to store data into	[out]
 * len:		Length of data to read		[in]
 *
 * Receives data from socket, storing it in buff.  Returns 0 on failure, 1 on success
 **/
int recvall(int sock, char *buff, int len){
	mem0(buff);

	int pos = 0, curr = 0, left = len, err = 0;

	while(pos < len){
		curr = recv(sock, buff+pos, left, 0);

		if(curr != -1){
			pos += curr;
			left -= curr;
		} else{
			err = errno;

			if(!NetSockErrOk(err)){
				perror("recv()");
				return 0;
			}
		}
	}

	return 1;
}

/**
 * recvlenall()
 *
 * Same as recvall(), but receives the length of the data first, then the actual data.
 **/
int recvlenall(int sock, char *buff){
	int len = 0;

	char *tmp = mem(strlen(buff));
	mem0(tmp);

	if(!recvall(sock, tmp, 5)){
		free(tmp);

		return 0;
	}

	len = atoi(buff);

	sleep(1);

	if(!recvall(sock, buff, len))
		return 0;

	return 1;
}

#endif
