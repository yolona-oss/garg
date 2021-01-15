#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#define MAX_GAMES 1000

#define MAX_EXCEPTIONS  1000
#define MAX_INCLUSIONS  MAX_EXCEPTIONS

struct game {
	int id;
	char *name;
	char *location;
	char *starPoint;
};

extern char *gameName;
extern struct game Game[MAX_GAMES+1];
extern char **confPathes;

extern char **exceptionName;
/* extern char *exceptionPath[MAX_EXCEPTIONS]; */
/* extern char *inclusions[MAX_INCLUSIONS]; */

#endif
