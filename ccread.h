#ifndef _CCREAD_H_
#define _CCREAD_H_

#include "global.h"

#define DEF_CACH_PTH ".cache/ga-org.conf"

#define _config_err(cfg) warn("Reading cache file: \"%s\" error:%d - %s", config_error_file(cfg), config_error_line(cfg), config_error_text(cfg));

int isCached(const char *path, struct Game_rec *Game);

void trimCache(const char *path);

char *findConfigFile(void);
char *initCacheFile(void);

int deleteGameEntryFromFile(const char *path, int n);

struct Game_rec *readGameEntryFromFile(const char *path, int n);
int readGameEntriesCountFromFile(const char *path);
int writeGameEntryToFile(const char *path, struct Game_rec *GE);

int readExceptions(const char *confPath);
int readConfig(void);

int writeCache(struct Game_rec **Games);
struct Game_rec **readCache(struct Game_rec **Games);

#endif
