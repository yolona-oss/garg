#ifndef _MAIN_H_
#define _MAIN_H_

extern char *argv0;

char *searchSP(const char *location);
int findGames(const char *path, int id);
int scan(const char *path);

void usage();

#endif
