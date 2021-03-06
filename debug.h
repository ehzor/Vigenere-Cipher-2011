#ifndef __DEBUG_H
#define __DEBUG_H

#include "global.h"

#include <time.h>
#include <sys/time.h>

// Make 0 on release
#define DEBUG 1

/**
 * If we want to debug code, then allow this function.
 * #if...#endif is outside of #define due to C being picky.
 *
 * If DEBUG != 1, then just do a regular printf().
 **/
#if DEBUG==1
	#define D(x) do {								\
		struct timeval tmp;							\
		struct timezone tz;							\
		gettimeofday(&tmp,&tz);							\
		printf ("%ld.%06ld ", tmp.tv_sec, tmp.tv_usec);				\
		printf ("[%s:%d] (%s)\t ", __FILE__, __LINE__, __FUNCTION__);		\
		printf x;								\
		printf ("\n");								\
	} while (0)
#else
	#define D(X) do {			\
				printf x;	\
				printf("\n");	\
	 } while(0)
#endif

/**
 * Helper functions for bottleneck() usage.
 **/
struct timeval gettime(){
	struct timeval tmp;
	struct timezone tz;

	if(gettimeofday(&tmp, &tz) != 0)
		D(("gettimeofday() failed."));

	return tmp;
}

/**
 * bottleneck()
 * s:		Starting timeval struct (start time)	[in]
 * e:		Ending timeval struct (end time)	[in]
 * process:	Name of process clocked			[in]
 *
 * Displays the seconds and milliseconds it took to compute a process.
 *
 * Only shows when DEBUG == 1
 **/
void bottleneck(struct timeval* s, struct timeval* e, char *process){
	struct timeval res;
	long int msec = 0;

	if(e->tv_usec < s->tv_usec){
		int nsec = (e->tv_usec - s->tv_usec) / 1000000 + 1;
		e->tv_usec -= 1000000 * nsec;
		e->tv_sec += nsec;
	}

	if((e->tv_usec - s->tv_usec) > 1000000){
		int nsec = (e->tv_usec - s->tv_usec) / 1000000;
		e->tv_usec += 1000000 * nsec;
		e->tv_sec -= nsec;
	}

	res.tv_sec = e->tv_sec - s->tv_sec;
	res.tv_usec = e->tv_usec - s->tv_usec;

	if(res.tv_usec < 0)
		res.tv_usec *= -1;

	while(res.tv_usec > 1000000){
		res.tv_usec -= 1000000;
		res.tv_sec += 1;
	}

	while(res.tv_usec > 1000){
		res.tv_usec -= 1000;
		msec += 1;
	}

	D(("%s took %ld seconds, %ld milliseconds, %ld microseconds.", process, res.tv_sec, msec, res.tv_usec));
}

#endif
