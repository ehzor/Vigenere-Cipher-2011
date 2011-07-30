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
 * NetGetIP()
 * addr:	Address to get the IP of	[in]
 *
 * Converts addr into an IP, and returns it...else, returns NULL
 **/
char *NetGetIP(char *addr){
	struct hostent *h;
	struct in_addr a;

	if((h = gethostbyname(addr))){
		while(*h->h_addr_list){
			memcpy((char*)&a, *h->h_addr_list++, sizeof(a));

			D(("IP address of %s is %s", addr, inet_ntoa(a)));
			return inet_ntoa(a);
		}
	}

	return '\0';
}

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
int NetServerCreate(char *addr, int port){
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
	//net.sin_addr.s_addr = INADDR_ANY;
	inet_pton(AF_INET, NetGetIP(addr), &net.sin_addr);

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

	D(("Server initialized on %s:%d", addr, port));

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

	if(inet_pton(AF_INET, NetGetIP(ip), &net.sin_addr) == -1){
		perror("inet_pton()");
		close(sock);

		return 0;
	}

	if(connect(sock, (struct sockaddr*)&net, sizeof(net)) == -1){
		perror("connect()");
		close(sock);

		return 0;
	}

	D(("Established connection to %s:%d", ip, port));

	return sock;
}

/**
 * NetSend()
 * sock:	Socket FD to send data to	[in]
 * data:	Buffer of data to send to sock	[in]
 *
 * Sends all of the data to sock, returns 0 on failure, 1 on success.
 **/
int NetSend(int sock, char *data){
	// Length of data
	int len = strlen(data);

	D(("Sending %d bytes of data to socket %d:\n%s", len, sock, data));

	// Current position of data buffer
	int pos = 0;

	// Amount of data that was sent
	int sent = 0;

	// Amount of data left (by default, all of data)
	int left = len;

	// Error number
	int err = 0;

	// Padding for data
	char *tmp = mem(len + 4);
	mem0str(tmp);

	// Pad data, with up to 4 0's prepended (i.e.: 25 = 0025), followed by all of the data
	sprintf(tmp, "%04d%s", len, data);

	// Increment length by 4 (due to the padding)
	len += 4;

	// While we are currently not at the end of the buffer
	while(pos <= len){
		// Send some data through the wire (4 bytes at a time)
		sent = send(sock, tmp+pos, 4, 0);

		// Check if an error occured
		if(sent > 0){
			// Data was sent, so update positions
			pos += sent;
			left -= sent;
		} else{
			err = errno;

			// Some errors are okay, as they are non-fatal to the transmission
			if(!NetSockErrOk(err))
				return 0;
		}
	}

	// Empty temporary buffer
	mem0str(tmp);

	// Empty data buffer
	mem0str(&data);

	// Success
	return 1;
}

/**
 * NetRecv()
 * socket:	Socket to receive data from	[in]
 * buff:	Buffer to store data into	[out]
 *
 * Receives data from socket, storing it in buff.  Returns 0 on failure, 1 on success
 **/
int NetRecv(int sock, char *buff){
	// Just to make sure the buffer is empty
	mem0str(buff);

	// Current position in buffer (by the time this is used, we'll be at the fourth position)
	int pos = 4;

	// Amount of bytes read
	int read = 0;

	// Error number
	int err = 0;

	// Get the length of the buffer
	char *tmp = mem(4);
	mem0str(tmp);

	recv(sock, tmp, 4, 0);

	// Convert the string for buffer length to integer (stripping padding 0's as well), then free memory
	int len = atoi(tmp);

	// Re-allocate space for tmp buffer
	tmp = remem(len);

	mem0str(tmp);

	// Amount of data left to read
	int left = len;

	while(left > 0){
		read = recv(sock, tmp, 4, 0);

		if(read > 0){
			left -= read;
			pos += read;

//			D(("Read %d bytes, now at position %d, with %d bytes left.", read, pos, left));
//			D(("Data fetched: %s", tmp));
			strcat(buff, tmp);
		} else{
			err = errno;

			if(!NetSockErrOk(err)){
				perror("recv()");
				mem0(tmp);

				return 0;
			}
		}

		mem0str(tmp);
	}

	return 1;
}

/**
 * NetAccept()
 * sock:	Server socket to get accepts from	[in]
 *
 * Wrapper for accept().  Returns client socket on succes, 0 on failure.
 **/
int NetAccept(int sock){
	struct sockaddr_in client;

	int s = 0, len = sizeof(client);

	if((s = accept(sock, (struct sockaddr*)&client, &len)) == -1){
		perror("accept()");

		return 0;
	}

	D(("Accepted connection from %s", inet_ntoa(client.sin_addr)));

	return s;
}

#endif
