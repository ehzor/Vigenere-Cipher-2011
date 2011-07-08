/********************************************
 * Vigenere Cipher 2011
 *
 * Property of Zorveo Research (Eric Hansen).
 *
 * Vigenere Cipher:
 * http://en.wikipedia.org/wiki/Vigen%C3%A8re_cipher
 *
 * Example:
 * Plain-text: ENCRYPTION
 * Key:        KEYKEYKEYK
 * Encryption: ORABCNDMMX
 *
 * let n = 0 (base = 0)
 * 
 * P = plain-text, K = key, C = cipher-text
 *
 * ENC() = ENCRYPT
 * DEC() = DECRYPT
 *
 * ENC(P) = (Pn + Kn) % 26 (while (n+1) != end of P)
 * DEC(C) = (Cn - Kn) % 26 (while (n+1) != end of C)
 *
 * The original cipher only used the uppercase English alphabet for keys.
 * This program uses those, and offers a chance to use others.
 * MODULO 26 = uppercase alphabet (cipher standard)
 * MODULO 52 = MODULO 26 and lowercase alphabet
 * MODULO 94 = MODULO 52 and all other characters in ASCII
 *
 * All vc_* functions (i.e.: vc_key()) are public.
 *
 * MODULO value is based on number of characters in table[].
 *
 * Please note that the cipher is dependent on the table[]...
 *
 * Developed as a proof of concept, this is meant to re-evaulate this cipher,
 * as well as propose ideals on how to combat the OTP (one-time pad) cipher scheme.
 ***********************************************/
#ifndef __KEY_H
#define __KEY_H

// This is for uint64_t
#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "random.h"

/**
 * See mod() comments for details on this.
 *
 * TBL SIZE	| INCLUDES			| STATUS
 * -------------------------------------------------------
 * 26		| A-Z				| WORKS
 * 52		| A-Z, a-z			| WORKS
 * 94		| A-Z, a-z, rest of ASCII table | WORKS
 *
 * NOTES:
 * - 52 is harder to code as in ASCII scan code, it has two ranges, instead of just one.
 * It works better now than it did originally though.
 **/
int MODULO = 94;

/**
 * struct __key {}
 *
 * Used as a buffer for sending and storing the key.
**/
typedef struct __key {
	uint64_t hi;
	uint64_t lo;
} keybuff;

/**
 * lookup_table[]
 * Look up table for all of the possible values in the key (used for deciphering)
 *
 * Table is base-0
 *
 * Usage:
 *
 * Return the character at position (Pc + Kc) for enciphering, or (Cc - Kc) for deciphering.
 *
 * By definition, using a static table can break the purpose of OTP, but since the key itself
 * is different upon each key-gen, it's trivial to worry about the table being an issue.
 **/
const char table[94] = {
			// MODULO == 26
			'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
			// MODULO == 52
			'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
			// MODULO == 94
			' ', '!', '"', '#', '$', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':',
			// -- 78
			';', '<', '=', '>', '?', '@', '[', '\\', ']', '^', '_', '`', '{', '|', '}', '~'
};

/**
 * tbl_lookup()
 * i:	Character to look up in table[]	[in]
 *
 * Routine for looking up character 'i' in table[].  Returns index it is found in.
 **/
int tbl_lookup(char i){
	// Since 'i' is already used, use the other generic j (base 0)
	int j = 0;

	while(j < MODULO){
		// table[j] will be the same as the character given
		if(table[j] == i)
			break;

		// No other match, so increment j by 1
		j++;
	}

	// Since 'i' is assumed to be in the table no matter what, return the position
	return j;
}

/**
 * encihper()
 * p:	Position of plain-text character (assumes it is a char)	[in]
 * k:	Position of key character (assumes it is a char)	[in]
 *
 * Returns values of table[p+k] for encryption.
 **/
char encipher(char p, char k){
	// Index counters for both plain-text and key characters
	int pn = 0, kn = 0;

	pn = tbl_lookup(p);
	kn = tbl_lookup(k);

	int i = pn + kn;

	// Since table is base-0, we need to make sure we're still in the bounds of MODULO
	while(i >= MODULO){
		i -= MODULO;
	}

	return table[i];
}

/**
 * decipher()
 * c:	Cipher character	[in]
 * k:	Key character		[in]
 *
 * Decrypts a cipher back to plain text.
 *
 * Same thing as encipher() basically, except for it subtracts pn and kn.
 **/
char decipher(char c, char k){
	// Index counters for both cipher and key characters
	int cn = 0, kn = 0;

	cn = tbl_lookup(c);
	kn = tbl_lookup(k);

	/**
	 * table[index] = cipher_index - key_index
	 *
	 * Unlike in encipher(), due to math theorems, substitution MUST be this way.
	 **/
	int i = cn - kn;

	if(i < 0)
		i += MODULO;

	return table[i];
}

/**
 * ret_range()
 * min:	Minimum number val can be	[in]
 * max:	Maximum number val can be	[in]
 * val:	The number to evaluate		[in]
 *
 * Returns a number between min & max, based on the value (val).
 *
 * In future, hope to turn val into in/out, instead of just in.
 **/
int ret_range(int min, int max, int val){
	while((val < min) || (val > max)){
		if(val < min)
			val += min;
		else if(val > max)
			val -= max;
	}

	return val;
}

/**
 * key()
 * bytes_read:	Amount of bytes to read from /dev/urandom	[in]
 * key_out:	Buffer to store generated key.			[out]
 *
 * Reads /dev/urandom and generates an ASCII key from the data.
 *
 * Returns number of bytes read if successful, 0 if failed.
 *
 * NOTE: return of key() should always be the same as bytes_read!!!
 **/
static int vc_key(int bytes_read, char *key_out){
	int res, tmp, min, max = 0;

	// Allocate a big enough buffer to hold x bytes of data
	char *buff = (char*)malloc(sizeof(char) * bytes_read);
	URandom(bytes_read, buff);

	if(MODULO == 26){
		min = 65;
		max = 90;
	} else if(MODULO == 52){
		min = 65;
		max = 122;
	} else if(MODULO == 94){
		min = 32;
		max = 126;
	}

	tmp = 0;

	while(tmp < bytes_read){
		res = buff[tmp];

		/**
		 * MODULO 52 is a special case that requires additional checks.  26 & 104 don't have this issue.
		 **/
		if(MODULO != 52){
			while((res < min) || (res > max)){
				res = ret_range(min, max, res);
			}
		} else{
			/**
			 * Only way I can get this to work...
			 * ASCII 65 - 90: A-Z
			 * ASCII 97 - 122: a-z
			 * We need to check to see if we are between 91 & 97, or if we've went over or under.
			 *
			 * if() block for this don't work for some reason...
			 **/
			while(((res > 'Z') && (res < 'a')) || ((res < min) || (res > max))){
				if((res > 'Z') && (res < 'a'))
					res -= 9;

				res = ret_range(min, max, res);
			}
		}

		key_out[tmp] = res;

		tmp++;
	}

//D(("KEY: %s", key_out));

	free(buff);

	return tmp;
}

/**
 * encrypt()
 * p:		Plain-text to encrypt		[in]
 * k:		Key to use for encryption	[in]
 * buff:	Buffer to store encrypted data	[out]
 *
 * Encrypts given data using the key, and stores it in buff.
 *
 **/
void vc_encrypt(char p[], char k[], char *buff){
	// p & k will both be the same length
	int len = strlen(p);

	// Current position inside of p & k
	int i = 0;

	while(i < len){
		buff[i] = encipher(p[i], k[i]);

		i++;
	}

//D(("ebuff: %s", buff));
}

/**
 * decrypt()
 * c:		Cipher used to encrypt text	[in]
 * k:		Key used to encrypt text	[in]
 * buff:	Buffer to store decrypted data	[out]
 *
 * Decrypts cipher (using key), and stores it into buff.
 **/
void vc_decrypt(char c[], char k[], char *buff){
	// c & k will be the same length
	int len = strlen(c);

	// Current position in c & k
	int i = 0;

	// While not at the end of the cipher text
	while(i < len){
		buff[i] = decipher(c[i], k[i]);

		i++;
	}
//D(("dbuff: %s", buff));
}

#endif
