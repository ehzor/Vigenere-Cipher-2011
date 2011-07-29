/***************************************
 * This file is a simple wrapper to handle
 * both encrypting and decrypting data.
 *
 * This is to cut down on the repeated stuff
 * that is being constantly done during the exchange
 * and whatnot.
 ***************************************/
#ifndef __ZCRYPT_H
#define __ZCRYPT_H

#include "dh.h"
#include "vc.h"
#include "trim.h"

/**
 * depad()
 **/
void depad(char *s){
	char *p = s;
	int l = strlen(p), i = 0, j = 0;
	char *q = (char*)malloc(sizeof(char) * l);
	memset(q, '\0', l);
	
	while(i <= l){
		if(!isspace(p[i])){
			q[j] = (char)p[i];
			j++;
		}

		i++;
	}

	memmove(s, q, l+1);
}

void pad(char *s){
	char *p = s;
	char *t = (char*)malloc(sizeof(char) * 310);

	int l = strlen(p), i = 0, j = (310 - l);

	while(j != 310){
		strcat(t, " ");
	}

	strcat(p, t);

	memmove(s, p, (j+1));

	free(t);
}

/**
 * encrypt()
 *
 * p:		Plain text to encrypt		[in]
 * buff:	Buffer to hold encrypted text	[out]
 * vck:		Vignere Cipher key		[in]
 * dhk:		Dillie-Hellman key		[in]
 * dhs:		Dillie-Hellman secret		[in]
 *
 * Encrypts data first through the Viegnere Cipher, then thru Dillie-Hellman.
 *
 * Stores encrypted text into "buff".
 **/
void zencrypt(char *p, char *buff, char *vck, mpz_t dhs){
//gmp_printf("DHS:	%Zd\n", dhs);
	char *tmp = (char*)malloc(sizeof(char) * strlen(p));

	memset(tmp, '\0', sizeof(tmp));

	// First, we do the VG cipher
	vc_encrypt(p, vck, tmp);

	// Next, we do the D-H encryption
//	dh_encrypt(tmp, sztmp, dhs);
//	pad(sztmp);
//	memmove(buff, tmp, strlen(tmp) + 1);

	int i = strlen(tmp), j = 0;

	while(j <= i){
		buff[j] = tmp[j];
		j++;
	}

	buff[j] = '\0';

	free(tmp);
}

// Same stuff as encrypt(), just doing the action in reverse on the ciphertext (c)
void zdecrypt(char *c, char *buff, char *vck, mpz_t dhs){
//gmp_printf("DHS:	%Zd\n", dhs);
	int i = 0;

	mpz_t mtmp;
	mpz_init(mtmp);
D(("c = %s", c));
//	depad(c);
//	i = strlen(c);
//D(("i = %d (%s)", i, c));
//	char *tmp = (char*)malloc(sizeof(char) * i);
//	memset(tmp, '\0', i);

	// dh_decrypt() requires a mpz_t variable, so convert the string to one
//	str2mpz(c, mtmp);
//gmp_printf("dh_e = %Zd\n", mtmp);
	// First, undo the Dillie-Hellman encryption
//	dh_decrypt(mtmp, tmp, dhs);

	// Perform the Viegnere OTP cipher to decrypt the text
	//vc_decrypt(tmp, vck, buff);
	vc_decrypt(c, vck, buff);

	mpz_clear(mtmp);
//	free(tmp);
}

#endif
