#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "eprintf.h"

int g_qflag, g_dflag;
char *argv0;

static void
verr(const char *fmt, va_list ap)
{
	if (!g_qflag) {
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

		fflush(stderr);
	}
}

void
allerMsg(const char *fmt, ...)
{
	if (g_dflag) {
		va_list ap;
		
		va_start(ap, fmt);
		vfprintf(stdout, fmt, ap);
		va_end(ap);

		fflush(stdout);
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
evsnprintf(char *str, size_t size, const char *fmt, va_list ap)
{
	int ret;

	ret = vsnprintf(str, size, fmt, ap);

	if (ret < 0) {
		warn("vsnprintf:");
		return -1;
	} else if ((size_t)ret >= size) {
		warn("vsnprintf: Output truncated");
		return -1;
	}

	return ret;
}

int
esnprintf(char *str, size_t size, const char *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = evsnprintf(str, size, fmt, ap);
	va_end(ap);

	return ret;
}

void *
emalloc(size_t size)
{
	void *p = malloc(size);

	if (!p) {
		die("malloc of %u bytes failed:", size);
	}
		
	return p;
}

void *
ecalloc(size_t nmemb, size_t size)
{
	void *buf = calloc(nmemb, size);

	if (!buf) {
		die("malloc of %u bytes failed:", size);
	}

	return buf;
}

void *
erealloc(void *ptr, size_t size)
{
	void *buf = realloc(ptr, size);

	if (!buf) {
		die("malloc of %u bytes failed:", size);
	}

	return buf;
}

char
*estrdup(const char *str)
{
	char *p = strdup(str);

	if (!p) {
		die("Cant duplicate string \"%s\"", str);
	}

	return p;
}
