#ifndef _UTIL_H_

#define _UTIL_H_

extern char *argv0;
extern char *gameName;

void verr(const char *fmt, va_list ap);
void warn(const char *fmt, ...);
void die(const char *fmt, ...);

void printPP(char **pa, char *sep, int n);
void freePP(char **pa, int n);

int rmDupInArrOfPointers(char *pa[], int n);

int isExist(const char *path);
int isDirectory(const char *path);
int isOtherDirectory(const char *path, const char *root);
int isDotName(const char *name);
int isExecuteble(const char *path);
int isExcludeName(const char *name);
int isStartPoint(const char *file);

int getRPath(const char *filename, const char *root, char *rpath);

int editGameEntry(int id, const char *name, const char *location, const char *startPoint);
void printGameEntry(int id);

#endif
