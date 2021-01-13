#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "util.h"

char *argv0;

void
verr(const char *fmt, va_list ap)
{
	if (argv0 && strncmp(fmt, "usage", sizeof("usage") - 1)) {
		fprintf(stderr, "%s: ", argv0);
	}

	vfprintf(stderr, fmt, ap);

	if (fmt[0] && fmt[strlen(fmt) - 1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else {
		fputc('\n', stderr);
	}
}

void
warn(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	verr(fmt, ap);
	va_end(ap);
}

void
die(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	verr(fmt, ap);
	va_end(ap);

	exit(1);
}

int
rmDupInArrOfPointers(char *pa[], int n)
{ 
    if (n == 0 || n == 1)
        return n;
 
    char *uniq[n];
	char *current;

	int k=0;
	int i, j;
	for(i=0; i<n; i++) {
	   if( pa[i] == NULL ) {
			continue;
		}
		current = pa[i];
		uniq[k] = current;
		k++;
		for(j=i+1; j<n; j++) {
			if( current == pa[j] ) {
				pa[j] = NULL;
			}
		}
	}

	for (j = i = 0; i < n; i++) {
		if (i > k-1) {
			pa[i] = NULL;
		} else {
			pa[i] = uniq[j++];
		}
	}
 
    return k-1;
}
