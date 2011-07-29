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

/**
 * gen_E()
 * 
 * buff:	Buffer to store equation	[out]
 * base:	Base of exponents (G/B/A)	[in]
 * x:		Exponent (Cs/Ss)		[in]
 * m:		Modulus (P)			[in]
 *
 * Wrapper of sorts for mpz_powm_sec(), be more effective when vars (besides buff) are not needed.
 **/
void gen_E(mpz_t buff, mpz_t base, mpz_t x, mpz_t m){
	mpz_powm_sec(buff, base, x, m);
}

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

//	gmp_randstate_t grand;

//	gmp_randinit_default(grand);
//	gmp_randseed_ui(grand, rndseedkey(bit * log(2)));

	birandom(bit, P, 1);

//	gmp_randclear(grand);

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

//	gmp_randstate_t grand;

	mpz_init(gcd);

//	gmp_randinit_default(grand);
//	gmp_randseed_ui(grand, rndseedkey((bit * bit) * log(2)));

	while(1){
		birandom(bit, G, 0);

		mpz_gcd(gcd, G, p);

		if(mpz_cmp_ui(gcd, (unsigned long int)1) == 0)
			break;
	}

//	gmp_randclear(grand);

	mpz_clear(gcd);
}

/**
 * dh_encrypt()
 * str:		The string to convert (calls str2mpz() inside)		[in]
 * buff:	Buffer to store the encrypted text			[out]
 * sk:		Secret key created @ end of exchange			[in]
 * priv_key:	Private key generated at beginning of exchange		[in]
 * pub_key:	The prublic key (if server, then B, if client, then A)	[in]
 *
 * XOR encrypts the str into buffer, making it all integers
 **/
void dh_encrypt(char *str, char *buffer /* mpz_t buffer */, mpz_t sk){
	mpz_t tmp, base, buff;
	mpz_init(tmp);
	mpz_init(base);
	mpz_init(buff);

	uint64_t bufflen = strlen(str)*3 + 1;

	char *szbuff = (char*)malloc(bufflen);
	memset(szbuff, '\0', bufflen);

	szbuff[0] = '1';

	int j = 0;

	char sztmp[4];

	uint64_t i = 0, len = strlen(str);

	for(; i < len; i++){
		j = (int)str[i];

		sprintf(sztmp, "%03d", j);

		strcat(szbuff, sztmp);
	}

	str2mpz(szbuff, tmp);

	mpz_xor(buff, tmp, sk);

	mpz2str(buff, buffer);
	
	free(szbuff);

	mpz_clear(tmp);
	mpz_clear(base);
	mpz_clear(buff);
}

/**
 * dh_decrypt()
 *
 * enc:		The encrypted stream to decrypt		[in]
 * buffer:	Where to store the decrypted text	[out]
 * sk:		The established secret key		[in]
 * pub_key:	The user's public key for the session	[in]
 *
 * Reverses the process of dh_encrypt() to turn the text back into readable form.
 **/
void dh_decrypt(mpz_t enc, char *buffer, mpz_t sk){
	// Create needed variables and initialize them
	mpz_t tmp, base, buff;

	mpz_init(tmp);
	mpz_init(base);
	mpz_init(buff);

	char *szbuff = (char*)malloc(MEMBUFF + 1);
	char *sztmp = (char*)malloc(MEMBUFF + 1);

        memset(szbuff, '\0', MEMBUFF);
	memset(sztmp, '\0', MEMBUFF);

	// dh_encrypt() uses XOR for encryption, so we use XOR to decrypt
	mpz_xor(buff, enc, sk);

	mpz2str(buff, sztmp);

        int k = 0, n = 0;

        uint64_t i = 1, len = strlen(sztmp);

	if(strlen(sztmp)%3 == 1){
		strcpy(szbuff, "00");
	} else if(strlen(sztmp) % 3 == 2){
		strcpy(szbuff, "0");
	}

	strcpy(szbuff, sztmp);

	while(i <= (len - 3)){
		n = sztmp[i] - 48;
		n = 10*n + (sztmp[i+1] - 48);
		n = 10*n + (sztmp[i+2] - 48);

		i += 3;
		buffer[k] = (char)n;

		k++;
	}

	free(szbuff);
	free(sztmp);

	mpz_clear(tmp);
	mpz_clear(base);
	mpz_clear(buff);
}

/**
 * gen_S()
 * buffer:	Where to store the secret key	[out]
 * p:		Modulus limit (1 < S < p)	[in]
 *
 * Wrapper to handle generating the secret number server & client use.
 *
 * 1 < buffer < p
 *
 * Keeps looping (if needed) until condition is met.
 **/
void gen_S(mpz_t buffer){

}
