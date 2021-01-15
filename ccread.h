#ifndef _CCREAD_H_
#define _CCREAD_H_

int getConfigIndex(void);

int readExceptions(const char *confPath);
int readConfig(void);
int readCache(const char *cachePath);

int writeCache(const char *cachePath);

#endif
