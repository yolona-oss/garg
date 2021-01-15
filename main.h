#ifndef _MAIN_H_
#define _MAIN_H_

#define MAX_EXCEPTIONS  1000
#define MAX_INCLUSIONS  MAX_EXCEPTIONS
#define MAX_GAMES       1000

struct game {
	int id;
	char *name;
	char *location;
	char *starPoint;
};

extern struct game Game[MAX_GAMES+1];
extern char *argv0;
extern char *gameName;


char *searchSP(const char *location);
int findGames(const char *path, int id);
int scan(const char *path);

void usage();

#endif
