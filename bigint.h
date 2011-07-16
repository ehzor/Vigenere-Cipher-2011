#ifndef __BIGINT_H
#define __BIGINT_H

/*********************************
 * This is simply a wrapper for used GMP functions.
 *********************************/
#include <gmp.h>
#include "random.h" // Used to generate a random seed for birandom()

/**
 * KEEP str2mpz() & mpz2str() @ TOP!
 * These are stored here as they involve GMP-related stuff, and I'm too lazy to make another header.
 **/
/**
 * str2mpz()
 * b	The current buffer of text to convert		[in]
 * m	The mpz_t variable to store converted text	[out]
 *
 * Converts a char* of numbers to an mpz_t capable format, in base 10.
 **/
void str2mpz(char *b, mpz_t m){
	mpz_set_str(m, b, 10);
}

/**
 * mpz2str()
 * m	The mpz_t variable to conver to string	[in]
 * b	Buffer to store converted text		[out]
 *
 * Opposite of str2mpz(), converts a mpz_t number to a string...useful for transmitted over the wire.
 **/
void mpz2str(mpz_t m, char *b){
	mpz_get_str(b, 10, m);
}

/**
 * birandom()
 * length:	The length of the key (i.e.: if 1024-bit key, length = 1024).	[in]
 * buffer:	Pointer to where to store the bigint				[out]
 * prime_only:	1 if prime is required, 0 if not				[in]
 *
 * Generates a random number of size (length), and stores it in buffer.
 *
 * Buffer must already be init'ed.
 **/
void birandom(uint64_t length, mpz_t buffer, uint8_t prime_only){
	// Set up our variables to use
	mpz_t randnum;

	// Used for RNG
	gmp_randstate_t state;

	mpz_init(randnum);

	// mpz_init() for RNG
	gmp_randinit_default(state);

	/**
	 * We need to seed the RNG so we get something new each time its ran
	 * rndseedkey() [see random.h] is the closest I can think of to it
	 **/
	gmp_randseed_ui(state, rndseedkey(length)); // Change 'length' to something more random

	// Get a random number (bound by *length*), and store it in *randnum*
	mpz_urandomb(randnum, state, length);

	if(prime_only){
		/**
		 * There's two ways we can do this...
		 * 1.  mpz_nextprime()...which I've done.
		 * 2.  Do a while(randnum != prime) loop, and just keep generating random numbers...
		 *
		 * mpz_netxprime() is less likely to make a bigger hit on the CPU...
		 **/
		mpz_nextprime(randnum, randnum);
	}

	// Same thing as swap()...store the random number generated into buffer
	mpz_set(buffer, randnum);

	// Free resources (this is the reason why we can't just return randnum)
	gmp_randclear(state);
	mpz_clear(randnum);
}

#endif
