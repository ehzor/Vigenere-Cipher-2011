#ifndef __DEBUG_H
#define __DEBUG_H

// Make 0 on release
#define DEBUG 1

// If we want to debug code, then allow this function
#if DEBUG==1
	#define D(x) do {								\
		printf ("%s\t%s (%d)\t ", __FILE__, __FUNCTION__, __LINE__);		\
		printf x;								\
		printf ("\n");								\
	} while (0)
#else
	#define D(X) do { } while(0)
#endif

#endif
