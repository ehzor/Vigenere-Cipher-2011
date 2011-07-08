/******************************************
 * Diffie-Hellman Key Exchange
 *
 * This is used to transmit data or a network securely.
 * The server must be using D-H as well, or else transmission will not work.
 *
 * All dh_* functions (i.e.: dh_encrypt()) are public functions.
 *
 * Please see appropriate source files for network information.
 ******************************************/
#include "debug.h"
//#include "genrand.h"
#include "random.h"
#include <stdint.h>

int prime(uint32_t v){
//uint64_t randgen = getrand(0, 'Z', 42, 69, 2, 10);
//D(("randgen = %"PRIu64, randgen));
uint64_t randgen = sized_num(300);
D(("randgen_1 = %"PRIu64, randgen));

	int t = 3;
	int n = 9;

	if((v == 2) || ((v & 1) == 0))
		return 0;

	while(n < v){
		if((v % t) == 0)
			return 0;

		t += 2;
		n = t * t;
	}

	if(n == v)
		return 0;

	return 1;
}
