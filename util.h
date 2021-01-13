#ifndef _UTIL_H_

#define _UTIL_H_

extern char *argv0;

void verr(const char *fmt, va_list ap);
void warn(const char *fmt, ...);
void die(const char *fmt, ...);

int rmDupInArrOfPointers(char *pa[], int n);

#endif
