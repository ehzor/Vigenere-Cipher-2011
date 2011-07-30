/*****************************************************
 * Viegnere Cipher
 *
 * Vigenere Cipher:
 * http://en.wikipedia.org/wiki/Vigen%C3%A8re_cipher
 *
 * If n = position in data stream of text (base 0), then:
 *
 * P = plain-text, K = key, C = cipher-text
 *
 * ENCRYPTION: ENC(n) = (P[n] + K[n]) % MODULO
 * DECRYPTION: DEC(n) = (C[n] - K[n]) % MODULO
 *
 * MODULO is number of alphanumeric values possible in the key (26, 52, 94)
 *
 * While the table for the Viegnere Cipher is static, the key seed is randomized (see random.h).
 * Also, the same key is theoritically never used twice (/dev/random & /dev/urandom are safest bets).
 *
 * This uses a pseudo-one time pad system for crypting the text.  While the system is flawed, added security
 * is used in the form of Dillie-Hellman Key Exchange (see dh.h).  The key exchange will be established first,
 * which will then ecrypt the data for the cipher (including key).
 *******************************************************/
#ifndef __VC_H
#define __VC_H

#endif
