/******************************************
 * Diffie-Hellman Key Exchange
 *
 * This is used to transmit data or a network securely.
 * The server must be using D-H as well, or else transmission will not work.
 *
 * All dh_* functions (i.e.: dh_encrypt()) are public functions.
 *
 * Server = A
 * Client = B
 * Ss = Server secret (random number)   
 * Cs = Client secret (random number)
 * Ssk = Server secret key
 * Csk = Client server key
 *
 * Ssk has to be the same as Csk
 *
 * A = (G ^ Ss)(mod P)
 * B = (G ^ Cs)(mod P)
 *
 * Server sends A to client, client sends B to server.
 *
 * Ssk = (B ^ Sc)(mod P)
 * Csk = (A ^ Ss)(mod P)
 *
 * Ssk & Csk are used to encrypt & decrypt text between both parties.
 *
 * - Server creates Ss, P, G
 * - Client creates Cs; receives P and G from server
 * - Client creates B; sends to server
 * - Server creates A; sends to client
 * - Server creates secret key Ssk
 * - Client creates secret key Csk
 *
 * Please see appropriate source files for network information.
 ******************************************/
#include "debug.h"
#include "bigint.h"
#include "random.h"
#include <stdint.h>
#include <math.h>

struct DillieHellman {
	mpz_t P;
	mpz_t G;
	mpz_t Ss;
	mpz_t Cs;
	mpz_t Ssk;
	mpz_t Csk;
} dh;

/**
 * gen_P()
 * bit:	The bit length (1024, 2048, 4096, or 8192) of the key	[in]
 * P:	The buffer to store the variable			[out]
 *
 * The D-H KE requires a prime number be generated for modulo computations.
 *
 * Returns the prime number found.
 **/
void gen_P(uint64_t bit, mpz_t P){
//	tvstart = gettime();

	uint64_t size = bit / 2;

	gmp_randstate_t grand;

	gmp_randinit_default(grand);
	gmp_randseed_ui(grand, rndseedkey(bit * log(2)));

	birandom(bit, P, 1);

	gmp_randclear(grand);

//	tvend = gettime();
//	bottleneck(&tvstart, &tvend, "Generating P");
}

/**
 * gen_G()
 * bit:	The bit length (1024, 2048, 4096, or 8192) of the key	[in]
 * p:	The modulus (used to determine if GCD(g,p) == 1)	[in]
 * G:	Buffer to store the new variable			[out]
 *
 * G is the primitive root of P, which means that the greatest common denominator of G & P must be one.
 *
 * Once this is found, we inform the user.
 **/
void gen_G(int bit, mpz_t p, mpz_t G){
	mpz_t gcd;

	gmp_randstate_t grand;

	mpz_init(gcd);

	gmp_randinit_default(grand);
	gmp_randseed_ui(grand, rndseedkey((bit * bit) * log(2)));

	while(1){
		birandom(bit, G, 0);

		mpz_gcd(gcd, G, p);

		if(mpz_cmp_ui(gcd, (unsigned long int)1) == 0)
			break;
	}

	gmp_randclear(grand);

	mpz_clear(gcd);
}
