#ifndef _GAMEREC_H_
#define _GAMEREC_H_

/* funcs */
void grp_free(struct Game_rec *Game);
void grt_free(struct Gr_tab Game);

void gr_print(Game_rec *Game);
int gr_is_broken(Game_rec *Game);

int  gr_init(Game_rec *dst, const char *name, const char *location, const char *sp, const char *unistaller);
void gr_edit(Game_rec *dst, unsigned int play_time, const char *name, const char *gener, const char *location, const char *sp, const char *unistaller);

int gr_add(Game_rec newrec);
int gr_delete(int n);
int gr_is_dup(Game_rec rec);

int grcmp(Game_rec src, Game_rec dst);
Game_rec *grcpy(Game_rec *dst, Game_rec *src);
Game_rec *grdup(Game_rec *rec);

#endif
