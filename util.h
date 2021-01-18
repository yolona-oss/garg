#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdarg.h>

#define __zero(P) memset((void *)(P), 0, sizeof(*P))

#define HOME getenv("HOME")
#define CAT_WITH_HOME(s, p, t) sprintf(t, "%s/%s", HOME, s);\
							   p=(char *)calloc(strlen(t)+1, sizeof(char));\
							   strcpy(p, t);

//garbage
enum gameEntryErr {
	G_NONAME = 1,
	G_NOLOC  = 2,
	G_NOSP   = 4,
};

extern char *argv0;
extern int g_qflag, g_dflag;
extern char *userConf;

void verr(const char *fmt, va_list ap);
void allerMsg(const char *fmt, ...);
void warn(const char *fmt, ...);
void die(const char *fmt, ...);

void printPP(char **pp, char *sep, int n);
void freePP(char **pp, int n);
void freePSG(struct Game_rec *Game);
void freePPSG(struct Game_rec **Game);

int rmDupInPP(char **pp, int n);

int isExist(const char *path);
int isDirectory(const char *path);
int isOtherDirectory(const char *path, const char *root);
int isDotName(const char *name);
int isExecuteble(const char *path);
int isExcludeName(const char *name);
int isStartPoint(const char *file, const char *gameName);

int getRPath(const char *filename, const char *root, char *rpath);
int getLenOfPP(char **pp);

void printGameEntry(struct Game_rec *Game);
int isBrokenGameEntry(struct Game_rec *Game);
int addGameEntry(struct Game_rec *Game, const char *name, const char *location, const char *startPoint);
int editGameEntry(struct Game_rec *Game, const char *name, const char *location, const char *startPoint);
int countGameEntries(struct Game_rec **Games);

int add_game(Game_rec newrec);
int del_game(int id);

int gecmp(struct Game_rec *src, struct Game_rec *dst);
int gecpy(struct Game_rec *dst, struct Game_rec *src);

int isGameEntryUniq(struct Game_rec **ppGame, struct Game_rec *pGame);
#endif
