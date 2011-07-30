/************************************
 * This file is a global header file.
 *
 * Stuff that'll be used numerous times, goes here.
 ************************************/
#ifndef __GLOBAL_H
#define __GLOBAL_H

// Various includes
//	System files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <stdint.h>

//		Used for GMP, should be in /usr/include/ after installing
#include <gmp.h>

//	Program-specific files
#include "debug.h"

// Defines needed
#define MEMBUFF 8192

/** Macro wrapper for strcmp() since its asinine otherwise...
  *
  * Usage: if(streq(..., ...)){ ... } = if(!strcmp(..., ...)){ ... }
**/
#define streq(a, b) !strcmp(a, b)
#define strneq(a, b, n) !strncmp(a, b, n)

// Functions to help ease programming

/**
 * mem0()
 * data:	The data to zero-out	[in/out]
 *
 * Calls memset() on data...just simply a wrapper.
 **/
void mem0(void *data){
	D(("Freeing %d bytes of memory.", sizeof(data)));

	memset(&data, 0, sizeof(data));
}

/**
 * mem0str()
 *
 * Same as mem0(), but used for char* variables.
 **/
void mem0str(char *data){
	D(("Freeing %d bytes of memory.", strlen(data)));

	memset(data, '\0', strlen(data));
}

/**
 * mem()
 * size:	The size of data to allocate	[in]
 *
 * Wrapper for malloc().  Works for char* only as it's the only time I seem to use it.
 **/
char *mem(int size){
	D(("Allocating %d bytes of memory.", (sizeof(char) * size)));

	return (char*)malloc(sizeof(char) * size);
}

/**
 * remem()
 *
 * Same as mem(), just expands or shrinks the memory space for the variable.
 **/
char *remem(int size){
	D(("Reallocating %d bytes of memory.", (sizeof(char) * size)));

	return (char*)realloc(NULL, sizeof(char) * size);
}

#endif
