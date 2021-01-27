#ifndef _UTIL_H_
#define _UTIL_H_

#define __zero(P) memset((void *)(P), 0, sizeof(*P))

#define INT_MAX_DIG      (int)round(log10(INT_MAX + 0.5))
#define UINT_MAX_DIG     (int)round(log10(UINT_MAX + 0.5))

#include "list.h"

//vars
extern char *argv0;
extern int g_qflag, g_dflag;
extern node_t *g_exceptions;
extern node_t *g_inclusions;

//func
char *cat_fnames(const char *f, const char *s);

void pp_nfree(char **pp, int n);
void pp_free(char **pp);

int pp_delete_dup(char **pp, int n);
int pp_get_len(char **pp);

int isExist(const char *path);
int isDirectory(const char *path);
int isOtherDirectory(const char *path, const char *root);
int isDotName(const char *name);
int isExecuteble(const char *path);
int isExcludeName(const char *name);
int isStartPoint(const char *file, const char *gameName);
int isUninstaller(const char *file_path, const char *null);

#endif
