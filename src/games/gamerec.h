#ifndef _GAMEREC_H_
#define _GAMEREC_H_

#include <time.h>

/* structs */
typedef struct Game_rec game_t;
typedef struct Gr_tab game_tab_t;
typedef struct Gr_prop game_prop_t;

struct Gr_prop {
	unsigned location: 1;    /* 0 - normal, 1 - incorrect path */
	unsigned icon: 1;        /* 0 - normal, 1 - incorrect path */
	unsigned start_point: 1; /* 0 - normal, 01 - incorrect  path, 10 - unexecuteble */
	unsigned uninstaller: 1; /* 0 - normal, 01 - incorrect  path, 10 - unexecuteble */
};

struct Game_rec {
	unsigned short id;      /* necessary */
	struct tm play_time;    /* necessary */ /* a total of time playing */
	struct tm last_time;    /* necessary */ /* last launch time */
	char *name;             /* necessary */
	char *icon;
	char *gener;
	char *location;         /* necessary */
	char *start_point;      /* necessary */
	char *start_argv;
	char *uninstaller;
	struct Gr_prop properties; /* necessary */
};

struct Gr_tab {
	int ngames;         /* using cells */
	int max;            /* allocated cells */
	struct Game_rec *game_rec;
};

/* consts */
enum { GR_INIT = 1, GR_GROW = 2 };

/* funcs */
int run_game(int id);

void grp_free(game_t *Game);
void grt_free(game_tab_t Game);

void gr_print(game_t *Game);

void check_gr_tab(void);

game_t *gr_init(const char *name, const char *location, const char *sp, const char *unistaller, const char *icon);
void gr_edit(game_t *dst, unsigned int play_time, const char *name, const char *gener, const char *location, const char *sp, const char *unistaller);

int gr_add(game_t *newrec);
int gr_delete(int id);
int gr_is_dup(game_t rec);
game_t *grt_find(int id);

/* utils */
int grcmp(game_t src, game_t dst);
game_t *grcpy(game_t *dst, game_t *src);
game_t *grdup(game_t *rec);

#endif
