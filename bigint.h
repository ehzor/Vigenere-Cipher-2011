#ifndef __BIGINT_H
#define __BIGINT_H

/*********************************
 * This is simply a wrapper for used GMP functions.
 *********************************/
#include <gmp.h>
#include "random.h" // Used to generate a random seed for birandom()

/**
 * birandom()
 * length:	The length of the key (i.e.: if 1024-bit key, length = 1024).	[in]
 * buffer:	Pointer to where to store the bigint				[out]
 *
 * Generates a random number of size (length), and stores it in buffer.
 *
 * Buffer must already be init'ed.
 **/
void birandom(uint64_t length, mpz_t buffer){
	mpz_t randnum;

	gmp_randstate_t state;

	mpz_init(randnum);

	gmp_randinit_default(state);
	gmp_randseed_ui(state, rndseedkey(length)); // Change 'length' to something more random

	mpz_urandomb(randnum, state, length);
// ??	mpz_nextprime(

	mpz_set(buffer, randnum);

	gmp_randclear(state);
	mpz_clear(randnum);
}

#endif
