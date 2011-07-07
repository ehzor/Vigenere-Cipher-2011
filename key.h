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
 * ENC(P) = Pn + Kn (while (n+1) != end of P)
 * DEC(C) = Cn - Kn (while (n+1) != end of C)
 *
 * The original cipher only used the uppercase English alphabet for keys.
 * This program uses those, and offers a chance to use others.
 * MODULO 26 = uppercase alphabet (cipher standard)
 * MODULO 52 = MODULO 26 and lowercase alphabet
 * MODULO 94 = MODULO 52 and all other characters in ASCII
 *
 * MODULO value is based on number of characters in table[].
 *
 * Developed as a proof of concept, this is meant to re-evaulate this cipher,
 * as well as propose ideals on how to combat the OTP (one-time pad) cipher scheme.
 ***********************************************/
#ifndef __KEY_H
#define __KEY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"

/**
 * See mod() comments for details on this.
 *
 * TBL SIZE	| INCLUDES			| STATUS
 * -------------------------------------------------------
 * 26		| A-Z				| WORKS
 * 52		| A-Z, a-z			| WIP
 * 94		| A-Z, a-z, rest of ASCII table | WORKS
 *
 * NOTES:
 * - 52 is harder to code as in ASCII scan code, it has two ranges, instead of just one.
 * It works better now than it did originally though.
 **/
#define MODULO	94

/**
 * lookup_table[]
 * Look up table for all of the possible values in the key (used for deciphering)
 *
 * Table is base-0
 *
 * Usage:
 *
 * Return the character at position (Pc + Kc) for enciphering, or (Cc - Kc) for deciphering.
 **/
const char table[94] = {
			// MODULO == 26
			'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
			// MODULO == 52
			'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
			// MODULO == 94
			' ', '!', '"', '#', '$', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':',
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

	char v;

	// Maximum value that i is bound to (base-0 solution)
	int bound = MODULO - 1;

	pn = tbl_lookup(p);
	kn = tbl_lookup(k);

	int i = pn + kn;

	// Since table is base-0, we need to make sure we're still in the bounds of MODULO
	while(i >= MODULO){
		i -= MODULO;
	}

	v = table[i];

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
 * key_index()
 * a:		ASCII/char number of cipher			[in]
 * b:		ASCII/char number of key			[in]
 * base:	Multiply modulus by base to increase index	[in]
 *
 * Returns modulus of a & b (modulo 26), giving position to use in key.
 * If base is > 1, then modulus will be (mod * base).
 *
 * Cipher requires modulo 26, but possibly could change this as the original
 * cipher would only use A-Z...this will use all printable ASCII characters.
 **/
static int key_index(int a, int b/*, int base = -1*/){
	// Get the sum of a & b for MODULO (more useful if self-made MODULO code is used)
	int root = (a + b);

	/**
	 * Self-implementation of MODULO code
	 * root % MODULO can be used, as this might cause more cycles than %
	 * Implemented for the fun of it.
	 **/
	int modulo = root - (MODULO * (root / MODULO));
	//int modulo = root % MODULO;
/*
	if(base > 1)
		modulo *= base;
*/
	return modulo;
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
#if MODULO != 52
	while((val < min) || (val > max)){
		if(val < min)
			val += min;
		else if(val > max)
			val -= max;
	}
#else
	while((val < min) || (val > max)){
		while((val > 90) && (val < 97)){
			val -= 97;
		}

		while(val > 122){
			val -= 122;
		}

		if(val < min)
			val += min;
		else if(val > max)
			val -= max;
	}
#endif
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
static int key(int bytes_read, char *key_out){
	FILE *fp;
	int len, res, tmp, min, max = 0;
	char *buffer;

	fp = fopen("/dev/urandom", "r");
	if(!fp){
		printf("Unable to open /dev/random!\n");
		return 0;
	}

	buffer = (char*)malloc(sizeof(char)*bytes_read);

	if(!buffer){
		printf("Unable to allocate enough memory!\n");
		return 0;
	}

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
		res = fgetc(fp);

		while((res < min) || (res > max)){
			res = ret_range(min, max, res);
		}

		sprintf(buffer, "%s%c", buffer, res);

		tmp++;
	}

	memcpy(key_out, buffer, bytes_read);

	fclose(fp);
	free(buffer);

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
void encrypt(char p[], char k[], char *buff){
	// p, k and buff will all be the same length
	int len = strlen(k);

	// Current position inside of p & k
	int i = 0;

	char *buffer = (char*)malloc(sizeof(char)*len);

	while(i < len){
		// buffer = buffer + encrypted character(p,k)
		sprintf(buffer, "%s%c", buffer, encipher(p[i], k[i]));

		i++;
	}

	// Copy buffer to buff, as we have to free up memory (good practice)
	strcpy(buff, buffer);

	free(buffer);
}

/**
 * decrypt()
 * c:		Cipher used to encrypt text	[in]
 * k:		Key used to encrypt text	[in]
 * buff:	Buffer to store decrypted data	[out]
 *
 * Decrypts cipher (using key), and stores it into buff.
 **/
void decrypt(char c[], char k[], char *buff){
	// c & k will be the same length
	int len = strlen(c);

	// Current position in c & k
	int i = 0;

	char *buffer = (char*)malloc(sizeof(char)*len);

	// While not at the end of the cipher text
	while(i < len){
		// buffer = buffer + decrypted character(c,k)
		sprintf(buffer, "%s%c", buffer, decipher(c[i], k[i]));

		i++;
	}

	memcpy(buff, buffer, len);

	free(buffer);
}

#endif
