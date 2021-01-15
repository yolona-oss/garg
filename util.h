#ifndef _UTIL_H_

#define _UTIL_H_

#define HOME getenv("HOME")
#define CAT_HOME(s, p, t) sprintf(t, "%s/%s", HOME, s);\
						  p=(char *)malloc(strlen(t)+1);\
						  strcpy(p, t);
//mb bitf instead
enum gameEntryErr {
	G_NONAME = 1,
	G_NOLOC  = 2,
	G_NOSP   = 4,
};

extern char *argv0;
extern char *gameName;
extern int qflag, dflag;
extern char *userConf;

void verr(const char *fmt, va_list ap);
void allerMsg(const char *fmt, ...);
void warn(const char *fmt, ...);
void die(const char *fmt, ...);

void printPP(char **pp, char *sep, int n);
void freePP(char **pp, int n);
void freeSG(struct game Game[]);

int rmDupInArrOfPointers(char *pa[], int n);

int isExist(const char *path);
int isDirectory(const char *path);
int isOtherDirectory(const char *path, const char *root);
int isDotName(const char *name);
int isExecuteble(const char *path);
int isExcludeName(const char *name);
int isStartPoint(const char *file);

int getRPath(const char *filename, const char *root, char *rpath);
int getLenOfPP(char **pp);

void printGameEntry(int id);
int checkGameEntry(int id);
int editGameEntry(int id, const char *name, const char *location, const char *startPoint);
void wipeGameEntry(int id);

int gecmp(struct game src, struct game dst);
int gecpy(struct game dst, struct game src);

#endif
