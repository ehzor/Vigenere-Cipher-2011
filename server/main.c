#include "../network.h"
#include "../dh.h"
#include "../vc.h"
#include <signal.h>

#define MEMBUFF (sizeof(char) * 8193)

void sigchld_handler(int s){
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

int main(int argc, char *argv[]){
	/** Server generates P & G for both server & client, sending it to the client...so, do P & G generation in while() loop after connect detect. **/
	// Allocate enough space for sizeof(char) * (bits + 1) [+1 to compensate for possible \0]
	char *szP = (char*)malloc(MEMBUFF);
	char *szG = (char*)malloc(MEMBUFF);
	char *szA = (char*)malloc(MEMBUFF);
	char *szB = (char*)malloc(MEMBUFF);
	char *buff = (char*)malloc(MEMBUFF);

	mpz_t P, G, Ss, B, A, Ssk;

	mpz_init(A);
	mpz_init(B);
	mpz_init(P);
	mpz_init(G);
	mpz_init(Ss);
	mpz_init(Ssk);	
/**
	mpz_t mP;
	mpz_t G;

	mpz_init(mP);
	mpz_init(G);

	gen_P(1024, P);
D(("P = %s", P));
gen_G(1024, mP, P);
D(("G = %s", P));
mpz_clear(p1);
**/
	int serverfd, connfd, len;

	struct addrinfo hints, *sinfo, *p;

	struct sockaddr_storage client_addr;

	socklen_t sin_size;

	struct sigaction sa;

	int yes = 1;
	int rv = 0;

	char buffer[65535] = {'\0'};

	char *host = NULL;
	char *port = (char*)malloc(sizeof(char) * 5); // sizeof(char) * (digits) [65535 = 5 digits]

	char srcip[INET6_ADDRSTRLEN];

	if(argc == 3){
		host = (char*)malloc(sizeof(char) * strlen(argv[1]));

		sprintf(host, "%s", argv[1]);
		sprintf(port, "%s", argv[2]);
	} else {
		host = (char*)malloc(sizeof(char) * strlen("127.0.0.1"));

		sprintf(host, "0.0.0.0");

		if(argc == 2)
			sprintf(port, "%s", argv[1]);
		else
			sprintf(port, "4309");
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if((rv = getaddrinfo(host, port, &hints, &sinfo)) != 0){
		D(("getaddrinfo(%d): %s", rv, gai_strerror(rv)));

		return 1;
	}

	for(p = sinfo; p != NULL; p = p->ai_next){
		if((serverfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
			perror("server: socket");
			continue;
		}

		setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if(bind(serverfd, p->ai_addr, p->ai_addrlen) == -1){
			close(serverfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if(!p){
		D(("Server was unable to bind... %s:%s", host, port));
		return 1;
	} else
		D(("Server is bound to %s:%s", host, port));

	freeaddrinfo(sinfo);

	if(listen(serverfd, 10) == -1){
		perror("listen");
		return 1;
	}

	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

	if(sigaction(SIGCHLD, &sa, NULL) == -1){
		perror("sigaction");
		return 1;
	}

	while(1){
		sin_size = sizeof(client_addr);

		connfd = accept(serverfd, (struct sockaddr*)&client_addr, &sin_size);

		if(connfd == -1){
			perror("accept");
			continue;
		}

		inet_ntop(client_addr.ss_family, in_addr((struct sockaddr*)&client_addr), srcip, sizeof(srcip));

		D(("Accepted new connection from %s", srcip));

		if(!fork()){
			close(serverfd);

			gen_P(1024, P);
			gen_G(1024, P, G);

birandom(1024, Ss, 0);
			mpz2str(P, szP);
			mpz2str(G, szG);


D(("Sending P (%s)", szP));
			sendall(connfd, szP);
D(("Sending G (%s)", szG));
			sendall(connfd, szG);

			recv(connfd, buff, MEMBUFF, 0);

			str2mpz(buff, B);
D(("Received B (%s)", buff));

	mpz_powm_sec(A, G, Ss, P);
mpz2str(A, szA);
D(("Sending A (%s)", szA));
			sendall(connfd, szA);

mpz_powm_sec(Ssk, B, Ss, P);
mpz2str(Ssk, buff);
D(("Secret key = %s", buff));
			close(connfd);

			exit(0);
		}

		close(connfd);
	}

	close(serverfd);

	free(buff);
	free(szP);
	free(szG);
	free(szB);
	free(szA);

	free(host);
	free(port);

	mpz_clear(A);
	mpz_clear(B);
	mpz_clear(G);
	mpz_clear(P);
	mpz_clear(Ss);
	mpz_clear(Ssk);

	return 0;
}
