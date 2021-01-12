#ifndef _MAIN_H_
#define _MAIN_H_

#define MAX_GAME_NAME 1000
#define MAX_EXCLUDES  1000
#define MAX_GAMES     1000

extern char *argv0;

int isDirectory(char *path);
int isOtherDirectory(char *path, char *root);
int isDotName(char *name);
int isExecuteble(char *path);

int getRPath(char *filename, char *root, char *rpath);
int getPathDepth(char *path);

int checkStartPoint(char *path);

char *searchInDir(char *root, int depth);
int scan(char *path);

static void usage();

#endif

