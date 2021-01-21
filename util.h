#ifndef _UTIL_H_
#define _UTIL_H_

#define __zero(P) memset((void *)(P), 0, sizeof(*P))

//vars
extern char *argv0;
extern int g_qflag, g_dflag;
extern char **exceptionName;

//func
char *cat_fnames(const char *f, const char *s);

void pp_free(char **pp, int n);

int pp_delete_dup(char **pp, int n);
int pp_get_len(char **pp);

int isExist(const char *path);
int isDirectory(const char *path);
int isOtherDirectory(const char *path, const char *root);
int isDotName(const char *name);
int isExecuteble(const char *path);
int isExcludeName(const char *name);
int isStartPoint(const char *file, const char *gameName);

#endif
