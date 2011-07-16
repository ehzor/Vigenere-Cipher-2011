#include "../network.h"
#include "../dh.h"
#include "../vc.h"

#define MEMBUFF (sizeof(char) * 8193)

int main(int argc, char *argv[]){
	int sockfd, nbytes;
	char *buff = (char*)malloc(sizeof(char) * 8193); // sizeof(char) * (bits + 1)
	struct addrinfo hints, *serverinfo, *p;
	int rv;
	char srcip[INET6_ADDRSTRLEN] = {'\0'};

	char *szP = (char*)malloc(MEMBUFF);
	char *szG = (char*)malloc(MEMBUFF);
	char *szA = (char*)malloc(MEMBUFF);
	char *szB = (char*)malloc(MEMBUFF);

	mpz_t P, G, Cs, Csk, A, B;

	mpz_init(P);
	mpz_init(G);
	mpz_init(A);
	mpz_init(B);
	mpz_init(Cs);
	mpz_init(Csk);

	birandom(1024, Cs, 0);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if((rv = getaddrinfo("127.0.0.1", "4309", &hints, &serverinfo)) != 0){
		D(("getaddrinfo() failed: %d; %s", rv, gai_strerror(rv)));
		return 1;
	}

	for(p = serverinfo; p != NULL; p = p->ai_next){
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
			D(("socket() failed: %d", sockfd));
			continue;
		}

		if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
			perror("connect() failed");
			continue;
		}

		break;
	}

	if(!p){
		D(("Unable to connect, aborting client program."));
		return 1;
	}

	inet_ntop(p->ai_family, in_addr((struct sockaddr*)p->ai_addr), srcip, sizeof(srcip));

	D(("Connected to %s", srcip));

birandom(1024, Cs, 0);
mpz2str(Cs, buff);
D(("Cs = %s", buff));

	freeaddrinfo(serverinfo);

	if((rv = recv(sockfd, buff, MEMBUFF, 0)) == -1){
		perror("recv");
		return 1;
	}

D(("P = %s", buff));
str2mpz(buff, P);

	recv(sockfd, buff, MEMBUFF, 0);
D(("G = %s", buff));
str2mpz(buff, G);

	mpz_powm_sec(B, G, Cs, P);
mpz2str(B, szB);

	send(sockfd, szB, strlen(szB), 0);

	recv(sockfd, buff, MEMBUFF, 0);
D(("A = %s", buff));
str2mpz(buff, A);

mpz_powm_sec(Csk, A, Cs, P);
mpz2str(Csk, buff);
D(("Csk = %s", buff));

	close(sockfd);

	free(buff);
	free(szP);
	free(szG);
	free(szA);
	free(szB);

	mpz_clear(A);
	mpz_clear(B);
	mpz_clear(P);
	mpz_clear(G);
	mpz_clear(Cs);
	mpz_clear(Csk);

	close(sockfd);

	return 0;
}
