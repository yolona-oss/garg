#ifndef _MAIN_H_
#define _MAIN_H_

#define MAX_EXCEPTIONS  1000
#define MAX_INCLUSIONS  MAX_EXCEPTIONS
#define MAX_GAMES       1000

extern char *argv0;

int isExist(const char *path);
int isDirectory(const char *path);
int isOtherDirectory(const char *path, const char *root);
int isDotName(const char *name);
int isExecuteble(const char *path);
int isExcludeName(const char *name);

int getRPath(const char *filename, const char *root, char *rpath);
int getConfigIndex(void);

int readExceptions(const char *confPath);
int readConfig(void);
int readCache(const char *cachePath);

int writeCache(const char *cachePath);

int checkStartPoint(int id, const char *file);

int editGameEntry(int id, const char *name, const char *location, const char *startPoint);

int findGameStartPoint(int gc);
int searchGameStartPoint(int id, const char *location);
int findGameLocations(const char *path, int id);
int scan(const char *path);

static void usage();

#endif
