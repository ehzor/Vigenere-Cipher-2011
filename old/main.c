#include <unistd.h>
#include <getopt.h>

// Viegere Cipher
#include "vc.h"

// Dillie-Hellman Key Exchange
#include "dh.h"

// Probably not needed but meh
#include "random.h"

int main(int argc, char *argv[]){
	int keybits = 0;
	int choice = 0;
	int mod = 0;

	int txtpos = argc - 1; 	// argc-1 is because argc is base 1, argv is base 0...and we don't know how many arguments we'll get.

	char *text = NULL;

	if((strlen(argv[txtpos]) < 1) || (argc == 1)){
		printf("Usage:\n%s -h\n", argv[0]);

		return 0;
	}

	text = (char*)malloc(sizeof(char) * strlen(argv[txtpos]) + 1);

	while((choice = getopt(argc, argv, "k:m:h")) != -1){
		switch(choice){
			case 'k':
				keybits = atoi(optarg);
			break;

			case 'm':
				mod = atoi(optarg);
			break;

			case 'h':
			default:
				printf("\t ZKEY v0.1\n\n");
				printf("Usage: %s <args> <text>\n\n", argv[0]);
				printf("ARGS:\n");
				printf("-k ###: Key length in bits for encryption\n");
				printf("-m ###: Key length for cipher encrypting and decrypting\n");
				printf("text: Any text that you want encrypted and sent to someone else\n\n");
				printf("For -k, valid values are 1024, 4096 and 8192\n");
				printf("For -m, valid values are 26, 52 and 94\n\n");
				printf("Program is released as is.\n");
			break;
		}
	}

	// This could be a goto, but trying to stay away from that.
	if(!keybits || !mod){
		free(text);

		return 0;
	}

	sprintf(text, "%s", argv[txtpos]);

	if((keybits != 1024) && (keybits != 2048) && (keybits != 8192)){
		printf("Keybit length of %d is invalid.\n", keybits);
		return 0;
	}

	if((MODULO != 26) && (MODULO != 52) && (MODULO != 94)){
		printf("Modulo value %d is invalid.\n", MODULO);
		return 0;
	} else
		MODULO = mod;
gen_p(keybits);
	free(text);

	return 0;
}

/*
	char *ck;	// Key buffer (stores generated key)
	char *text;	// The text to cipher
	char *ebuff;	// Encryption buffer
	char *dbuff;	// Decryption buffer
	int mod = 0;

	if(argc == 3)
		modulo = atoi(argv[2]);
	else
		modulo = 94;

	MODULO = modulo;

	// Typecasting of (const char*) was done to stop an annoying warning
	int len = strlen((const char*)argv[1]);	// Length of the text (needed for correct data reading of /dev/random)

	// Allocate enough memory for both buffers
	text = (char*)malloc(sizeof(char)*len);
	ck = (char*)malloc(sizeof(char)*len);
	ebuff = (char*)malloc(sizeof(char)*len);
	dbuff = (char*)malloc(sizeof(char)*len);

	// Fetch the key (of length len), storing it inside of ck
	vc_key(len, ck);

	// Copy the user's text into buffer
	memcpy(text, argv[1], len);

	// Run encrypt and decrypt routines
	vc_encrypt(text, ck, ebuff);
	vc_decrypt(ebuff, ck, dbuff);

	printf("%s\n", ck);

	// Since we are decrypting, dbuff will be the same as text if successful
	if(strcmp(text, dbuff) != 0)
		printf("Error running algorithm.\nP: %s\nK: %s\nC: %s\nD: %s\n", text, ck, ebuff, dbuff);

uint64_t p = gen_p();
D(("p = %"PRIu64, p));
	// Free up memory...must always be at bottom
	free(text);
	free(ck);
	free(ebuff);
	free(dbuff);

	return 0;
}
*/

