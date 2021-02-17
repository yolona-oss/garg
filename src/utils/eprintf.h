#ifndef _EPRINTF_H_
#define _EPRINTF_H_

#include <stdarg.h>
#include <stddef.h>

/* funcs */
void allerMsg(const char *fmt, ...);
void warn(const char *fmt, ...);
void die(const char *fmt, ...);

int evsnprintf(char *str, size_t size, const char *fmt, va_list ap);
int esnprintf(char *str, size_t size, const char *fmt, ...);

void *emalloc(size_t size);
void *ecalloc(size_t nmemb, size_t size);
void *erealloc(void *ptr, size_t size);

char *estrdup(const char *str);

/* vars */
extern char *argv0;

#endif
