#ifndef _MAIN_H_
#define _MAIN_H_

/* functions */
int scan(const char *path);

/* variables */
typedef struct Game_rec Game_rec;
struct Game_rec {
	unsigned int id;
	unsigned int play_time; /* in minutes */
	char *name;
	char *gener;
	char *location;
	char *start_point;
	char *uninstaller;
};

struct Gr_tab {
	int ngames;         /* using cells */
	int max;            /* allocated cells */
	Game_rec *game_rec;
};

extern char *argv0;
extern int g_qflag, g_dflag;

extern char *userConf;
extern char *cachePath;

extern char **exceptionName;
extern char **exceptionPath;
extern char **inclusions;

extern struct Gr_tab gr_tab;

enum { GR_INIT = 1, GR_GROW = 2 };

#endif
