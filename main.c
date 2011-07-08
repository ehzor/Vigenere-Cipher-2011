#include "key.h"
#include "dh.h"
#include "random.h"

int main(int argc, char *argv[]){
prime(3);
	char *ck;	// Key buffer (stores generated key)
	char *text;	// The text to cipher
	char *ebuff;	// Encryption buffer
	char *dbuff;	// Decryption buffer

	int modulo = 0;

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

	// Free up memory...must always be at bottom
	free(text);
	free(ck);
	free(ebuff);
	free(dbuff);

	return 0;
}

