#include "network.h"

int main(int argc, char *argv[]){
	D(("Creating client socket..."));

	int socket = NetClientCreate(argv[1], atoi(argv[2]));

	if(!socket)
		return 0;

	D(("Socket created %d", socket));

	char *buff = mem(65535);

	NetRecv(socket, buff);
D(("-- buff = %s", buff));
//	mem0str(buff);
	NetRecv(socket, buff);
D(("-- buff = %s", buff));

	mem0str(buff);

	return 0;
}


