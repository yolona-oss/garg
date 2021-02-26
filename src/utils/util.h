#ifndef _UTIL_H_
#define _UTIL_H_

#include <math.h>
#include <limits.h>

#define __zero(P) memset((void *)(P), 0, sizeof(*P))

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define DIG(n)           (int)round(log10(n+0.5))
#define INT_MAX_DIG      (int)DIG(INT_MAX)
#define UINT_MAX_DIG     (int)DIG(UINT_MAX)
/* #define INT_MAX_DIG      (int)round(log10(INT_MAX + 0.5)) */
/* #define UINT_MAX_DIG     (int)round(log10(UINT_MAX + 0.5)) */

#define PERC_OF(n, p) n*p/100
#define IS_ODD(n) (((int)n/2)-((int)n/2))

#include "list.h"

//vars
extern char buf[4096];

extern char *argv0;
extern int g_qflag, g_dflag;

extern node_t *g_exceptions;
extern node_t *g_inclusions;

//func
char *cat_fnames(const char *f, const char *s);
char *cut(char *str, int width);

void pp_nfree(char **pp, int n);
void pp_free(char **pp);

int pp_delete_dup(char **pp, int n);
int pp_length(char **pp);
char **pp_sort(char **pp, int check(const char *));

int isExist(const char *path);
int isDirectory(const char *path);
int isOtherDirectory(const char *path, const char *root);
int isDotName(const char *name);
int isExecuteble(const char *path);
int isExcludeName(const char *name);
int isStartPoint(const char *file, const char *gameName);
int isUninstaller(const char *file_path, const char *null);

char *bprintf(const char *fmt, ...);

char *itoa(int n, int base);
int ctocp(char c, char *ret);

int getCountsOfDigits(int n);

#endif
