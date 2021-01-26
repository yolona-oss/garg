#ifndef _GAMEREC_H_
#define _GAMEREC_H_

/* structs */
typedef struct Game_rec Game_rec;

struct Gr_prop {
	unsigned location: 1;    /* 0 - normal, 1 - incorrect path */
	unsigned icon: 1;        /* 0 - normal, 1 - incorrect path */
	unsigned start_point: 1; /* 0 - normal, 01 - incorrect  path, 10 - unexecuteble */
	unsigned uninstaller: 1; /* 0 - normal, 01 - incorrect  path, 10 - unexecuteble */
};

struct Game_rec {
	unsigned short id;      /* necessary */
	unsigned int play_time; /* in minutes */
	char *name;             /* necessary */
	char *icon;
	char *gener;
	char *location;         /* necessary */
	char *start_point;      /* necessary */
	char *start_argv;
	char *uninstaller;
	struct Gr_prop properties;
};

struct Gr_tab {
	int ngames;         /* using cells */
	int max;            /* allocated cells */
	Game_rec *game_rec;
};

/* consts */
enum { GR_INIT = 1, GR_GROW = 2 };

/* funcs */
void grp_free(struct Game_rec *Game);
void grt_free(struct Gr_tab Game);

void gr_print(Game_rec *Game);

int  gr_get_props(Game_rec *grp, struct Gr_prop prop);
void gr_set_props(Game_rec *grp, struct Gr_prop *prop);

Game_rec *gr_init(const char *name, const char *location, const char *sp, const char *unistaller);
void gr_edit(Game_rec *dst, unsigned int play_time, const char *name, const char *gener, const char *location, const char *sp, const char *unistaller);

int gr_add(Game_rec *newrec);
int gr_delete(int id);
int gr_is_dup(Game_rec rec);

int grcmp(Game_rec src, Game_rec dst);
Game_rec *grcpy(Game_rec *dst, Game_rec *src);
Game_rec *grdup(Game_rec *rec);

#endif
