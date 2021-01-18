#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#define MAX_GAMES 1000

#define MAX_EXCEPTIONS  1000
#define MAX_INCLUSIONS  MAX_EXCEPTIONS

typedef struct Game_rec Game_rec;
struct Game_rec {
	int id;
	char *name;
	char *location;
	char *start_point;
};

struct Gr_tab {
	int ngames;
	int max;
	Game_rec *game_rec;
};

enum { GR_INIT = 1, GR_GROW = 2 };

extern char *cachePath;
extern struct Gr_tab gr_tab;

extern char **exceptionName;
/* extern char *exceptionPath[MAX_EXCEPTIONS]; */
/* extern char *inclusions[MAX_INCLUSIONS]; */

#endif
