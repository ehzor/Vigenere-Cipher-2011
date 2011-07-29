#include "../network.h"
#include "../zcrypt.h"

int main(int argc, char *argv[]){
	int sockfd, nbytes, bufflen;
	char *buff = (char*)malloc(sizeof(char) * MEMBUFF);
	struct addrinfo hints, *serverinfo, *p;
	int rv, vhkey, key;
	char srcip[INET6_ADDRSTRLEN] = {'\0'};
	int yes = 1;

	char *szP = (char*)malloc(sizeof(char) * PGLEN);
	char *szG = (char*)malloc(sizeof(char) * PGLEN);
	char *szA = (char*)malloc(sizeof(char) * MEMBUFF);
	char *szB = (char*)malloc(sizeof(char) * MEMBUFF);
	char *szCrypt = (char*)malloc(sizeof(char) * MEMBUFF);
	char *szVkey = (char*)malloc(sizeof(char) * VC_KEY);
	char *szVbuff = (char*)malloc(sizeof(char) * VC_BUFF);

	memset(buff,	'\0', MEMBUFF	);
	memset(szP,	'\0', PGLEN	);
	memset(szG,	'\0', PGLEN	);
	memset(szA,	'\0', MEMBUFF	);
	memset(szB,	'\0', MEMBUFF	);
	memset(szCrypt,	'\0', MEMBUFF	);
	memset(szVkey,	'\0', VC_KEY	);
	memset(szVbuff,	'\0', VC_BUFF	);

	mpz_t P, G, Cs, Csk, A, B, vkey;

	mpz_init(P);
	mpz_init(G);
	mpz_init(A);
	mpz_init(B);
	mpz_init(Cs);
	mpz_init(Csk);
	mpz_init(vkey);

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

		if(setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int)) < 0){
			perror("setsockopt()");
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

	freeaddrinfo(serverinfo);

	// Get the key size to use for the system
	bufflen = recvbufflen(sockfd);
	recvall(sockfd, buff, bufflen);
	//recv(sockfd, buff, bufflen, 0);
	key = atoi(buff);

	// Generate a random secret key used for the D-H algorithm
	birandom(key, Cs, 0);

	// Get P & G from the server
	bufflen = recvbufflen(sockfd);
	recvall(sockfd, buff, bufflen);
D(("P = %s", buff));
	//recv(sockfd, buff, bufflen, 0);
	str2mpz(buff, P);

	bufflen = recvbufflen(sockfd);
	recvall(sockfd, buff, bufflen);
D(("G = %s", buff));
	//recv(sockfd, buff, bufflen, 0);
	str2mpz(buff, G);

	// B = (G ^ Cs)(mod P)
	mpz_powm_sec(B, G, Cs, P);
	mpz2str(B, szB);

	// Tell the server what our B value is
	sendbufflen(sockfd, strlen(szB));
	sendall(sockfd, szB);

	// Get the server's A value
	bufflen = recvbufflen(sockfd);
	recvall(sockfd, buff, bufflen);
D(("A = %s", buff));
	//recv(sockfd, buff, bufflen, 0);
	str2mpz(buff, A);

	//
	 // Secret key = (A ^ Cs)(mod P)
	 // This is used to encrypt the data
	 ///
	mpz_powm_sec(Csk, A, Cs, P);

	// Get the Viegnere Cipher key strength (26, 54 or 96)
	bufflen = recvbufflen(sockfd);
	memset(buff, '\0', MEMBUFF);
	recvall(sockfd, buff, bufflen);
D(("VCKEY SIZE = %s", buff));
	//recv(sockfd, buff, bufflen, 0);
	MODULO = atoi(buff);
	memset(buff, '\0', MEMBUFF);

	// Get the Viegnere Cipher key from server and decrypt it
	bufflen = recvbufflen(sockfd);
	recvall(sockfd, buff, bufflen);
D(("VCKEY = %s", buff));
	//recv(sockfd, buff, bufflen, 0);
	str2mpz(buff,vkey);
	dh_decrypt(vkey, szVkey, Csk);

	// Send the username to the server
	memset(szVbuff, '\0', VC_BUFF);
	zencrypt("love", szVbuff, szVkey, Csk);
D(("User (%d) = %s", strlen(szVbuff), szVbuff));
	sendbufflen(sockfd, strlen(szVbuff));
	sendall(sockfd, szVbuff);

	// Send the password to the server
	memset(buff, '\0', sizeof(buff));
	zencrypt("godsex", buff, szVkey, Csk);
D(("Pass (%d) = %s", strlen(buff), buff));
	sendbufflen(sockfd, strlen(buff));
	sendall(sockfd, buff);

	// Get the server response
	memset(buff, '\0', strlen(buff));
	bufflen = recvbufflen(sockfd);
	recvall(sockfd, buff, bufflen);
	//recv(sockfd, buff, bufflen, 0);
	zdecrypt(buff, szVbuff, szVkey, Csk);
D(("Server responded with %s", szVbuff));

	close(sockfd);

	free(buff);
	free(szP);
	free(szG);
	free(szA);
	free(szB);
	free(szCrypt);
	free(szVkey);
	free(szVbuff);

	mpz_clear(A);
	mpz_clear(B);
	mpz_clear(P);
	mpz_clear(G);
	mpz_clear(Cs);
	mpz_clear(Csk);
	mpz_clear(vkey);

	close(sockfd);

	return 0;
}
