#include "network.h"
#include <pthread.h>

void *handle_client(void *sock){
	D(("\tInside thread"));

	int s = *((int*)sock);

	NetSend(s, "We are testing a send.");
	NetSend(s, "Testing another send.");

	D(("\tLeaving thread"));
}

int main(int argc, char *argv[]){
	fd_set active_fd, read_fd;

	int socket = NetServerCreate(argv[1], atoi(argv[2]));

	int i, status, client;

	pthread_t mthread;

	if(!socket){
		D(("Unable to create socket for server.  Exiting."));

		return 0;
	}

	FD_ZERO(&active_fd);
	FD_SET(socket, &active_fd);

	while(1){
		read_fd = active_fd;

		if(select(FD_SETSIZE, &read_fd, NULL, NULL, NULL) == -1){
			perror("select()");
			close(socket);

			return 0;
		}

		for(i = 0; i < FD_SETSIZE; ++i){
			if(FD_ISSET(i, &read_fd)){
				if(i == socket){
					//size = sizeof(client);

					if(!(client = NetAccept(socket))){
						perror("NetAccept() failed.");
						close(socket);

						return 0;
					}

					pthread_create(&mthread, NULL, handle_client, (void*)&client);
					pthread_join(mthread, NULL);

					FD_SET(client, &active_fd);
				} else{
					// do socket handling stuff here

					close(client);
					FD_CLR(client, &active_fd);
				}
			}
		}
	}

	return 0;
}
