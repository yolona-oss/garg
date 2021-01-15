#ifndef _CCREAD_H_
#define _CCREAD_H_

#define DEF_CACH_PTH ".cache/ga-org.conf"

char *findConfigFile(void);

char *initCacheFile(void);

int deleteGameEntryFromFile(const char *path, int n);

struct game *readGameEntryFromFile(const char *path, int id);
int readGameEntriesCountFromFile(const char *path);

int readExceptions(const char *confPath);
int readConfig(void);
int readCache(void);

void writeGameEntryToFile(const char *path, config_setting_t *stt, struct game GE);
int writeCache(void);

#endif
