#include "../network.h"
#include "../zcrypt.h"

#include <shadow.h>
#include <crypt.h>
#include <signal.h>

/** Used for LOGIN_NAME_MAX define **/
#include <bits/local_lim.h>

void sigchld_handler(int s){
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

const int key = 2048 / 2;
const int vhkey = 94;

/**
 * shadowauth()
 * u:	Username to authenticate	[in]
 * p:	Given password for user		[in]
 *
 * Authenticates user against /etc/shadow file.
 *
 * Retuns 1 on success, 0 on failure.
 **/
int shadowauth(const char *u, const char *p){
	// Shadow password structure (see shadow.h)
	struct spwd *spw = NULL;

	// salt = $#$....$ (i.e.: $1$abcdefg)
	char *salt = (char*)malloc(sizeof(char) * 11);
	memset(salt, '\0', 11);

	int ret = 0;

	// Populate shadow structure based on given username
	if((spw = getspnam(u)) != NULL){
		// sp_pwdp = entire password, salt is the first 11 characters
		strncat(salt, (spw->sp_pwdp), 11);

		// If we encrypt p with salt, and its the same as sp_pwdp, success
		if(streq(crypt(p, salt), spw->sp_pwdp))
			ret = 1;
	}

	free(salt);

	return ret;
}

int main(int argc, char *argv[]){
//D(("shadowauth(love,godsex) = %d", shadowauth("love", "godsex")));

	// How big is the next buffer going to be?
	int bufflen = 0;

	/** Server generates P & G for both server & client, sending it to the client...so, do P & G generation in while() loop after connect detect. **/
	// Allocate enough space for sizeof(char) * (bits + 1) [+1 to compensate for possible \0]
	char *szP  = (char*)malloc(PGLEN);
	char *szG  = (char*)malloc(PGLEN);
	char *szA  = (char*)malloc(ABLEN);
	char *szB  = (char*)malloc(ABLEN);
	char *buff = (char*)malloc(MEMBUFF);
	char *user = (char*)malloc(LOGIN_NAME_MAX);
	char *pw   = (char*)malloc(MEMBUFF);
	char *szVC = (char*)malloc(VC_BUFF);
	char *szVKey = (char*)malloc(VC_KEY);

	memset(szP,  '\0', PGLEN);
	memset(szG,  '\0', PGLEN);
	memset(szA,  '\0', ABLEN);
	memset(szB,  '\0', ABLEN);
	memset(buff, '\0', MEMBUFF);
	memset(user, '\0', LOGIN_NAME_MAX);
	memset(pw,   '\0', MEMBUFF);
	memset(szVC, '\0', VC_BUFF);
	memset(szVKey, '\0', VC_KEY);

	mpz_t P, G, Ss, B, A, Ssk, tmp;

	mpz_init(A);
	mpz_init(B);
	mpz_init(P);
	mpz_init(G);
	mpz_init(Ss);
	mpz_init(Ssk);	
	mpz_init(tmp);

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
		setsockopt(serverfd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int));

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

			memset(buff, '\0', MEMBUFF);

			// Send the key bit strength to the client
			sprintf(buff, "%d", key);
D(("Sending %s", buff));
			sendbufflen(connfd, strlen(buff));
D(("--sendbuff() passed."));
			sendall(connfd, buff);
D(("Buffer sent"));
			// Since otherwise this can cause problems, make sur buff is emptied again
			memset(buff, '\0', 4);

			// Generate P & G, as well as the sever's secret
			gen_P(key, P);
			gen_G(key, P, G);

			birandom(key, Ss, 0);

			// A = (G ^ Ss)(mod P)
			mpz_powm_sec(A, G, Ss, P);
			mpz2str(A, szA);

			// Convert P & G to wire-transferable format, then send them to client
			mpz2str(P, szP);
			mpz2str(G, szG);

			sendbufflen(connfd, strlen(szP));
			sendall(connfd, szP);

			sendbufflen(connfd, strlen(szG));
			sendall(connfd, szG);

			// Get the client's B value
			bufflen = recvbufflen(connfd);
			recvall(connfd, buff, bufflen);
D(("B = %s", buff));
			//recv(connfd, buff, bufflen, 0);
			str2mpz(buff, B);

			// Tell the client our A value
			sendbufflen(connfd, strlen(szA));
			sendall(connfd, szA);

			// We generate our secret key with the same formulas as A
			mpz_powm_sec(Ssk, B, Ss, P);

			// Tell the client the key size of the Viegnere Cipher (26, 54, or 96)
			sprintf(buff, "%d", vhkey);
			sendbufflen(connfd, strlen(buff));
			sendall(connfd, buff);

			vc_key(VC_KEY, szVKey);
			dh_encrypt(szVKey, buff, Ssk);
			sendbufflen(connfd, strlen(buff));
			sendall(connfd, buff);

			// Get the username from the client
			memset(buff, '\0', strlen(buff));
			bufflen = recvbufflen(connfd);
			recvall(connfd, buff, bufflen);
D(("USER = %s", buff));
			//recv(connfd, buff, bufflen, 0);
			zdecrypt(buff, user, szVKey, Ssk);

//D(("User = %s", user));

			memset(szVC, '\0', strlen(szVC));
			// Receive the cipher text of the user's password (encrypted with D-H)
			bufflen = recvbufflen(connfd);
			recvall(connfd, szVC, bufflen);
D(("PASS = %s", buff));
			//recv(connfd, szVC, VC_BUFF, 0);
			// Decrypt it to get the plain-text password from user
			zdecrypt(szVC, pw, szVKey, Ssk);
//D(("Pass = %s", pw));

memset(buff, '\0', strlen(buff));
			if(!shadowauth(user, pw))
				zencrypt("FAIL", buff, szVKey, Ssk);
			else
				zencrypt("OK", buff, szVKey, Ssk);
D(("buff = %s", buff));
			sendbufflen(connfd, strlen(buff));
			sendall(connfd, buff);

			//close(connfd);

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
	free(pw);
	free(szVC);
	free(szVKey);

	free(host);
	free(port);

	mpz_clear(A);
	mpz_clear(B);
	mpz_clear(G);
	mpz_clear(P);
	mpz_clear(Ss);
	mpz_clear(Ssk);
	mpz_clear(tmp);

	return 0;
}
