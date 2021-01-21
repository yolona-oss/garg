#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "util.h"
#include "eprintf.h"
#include "gamerec.h"

void
grp_free(Game_rec *Game)
{
	if (Game) {
		Game->id = -1707;
		if (Game->name)
			free(Game->name);
		if (Game->location)
			free(Game->location);
		if (Game->start_point)
			free(Game->start_point);
		free(Game);
	}
}

void
grt_free(struct Gr_tab grt)
{
	free(gr_tab.game_rec);
}

void
gr_print(Game_rec *Game)
{
	if (Game) {
		printf("\n######################################\n");
		printf(" id          - %d\n", Game->id);
		printf(" name        - %s\n", Game->name);
		printf(" location    - %s\n", Game->location);
		printf(" start point - %s\n", Game->start_point);
		printf("######################################\n\n");

		fflush(stdout);
	} else {
		warn("Cant print game record: record dose not exist");
	}
}

//TODO
int
gr_is_broken(Game_rec *Game)
{
	/* short eflag = 0; */

	if (!Game) {
		return -1;
	}

	if (!isExist(Game->location)) {
		/* warn("Game location dont exist"); */
		/* eflag |= G_NOLOC; */
		return 1;
	}
		
	if (!isStartPoint(Game->start_point, Game->name)) {
		/* warn("Game start point dont exist"); */
		/* eflag |= G_NOSP; */
		return 1;
	}

	if (!Game->name) {
		/* warn("Game dont hame name"); */
		/* eflag |= G_NONAME; */
		return 1;
	}

	return 0;
}

unsigned int
gr_make_id(const char *str)
{
	unsigned int ret = 0;

	if (!str) {
		return -1;
	}

	for (int i = 0; str[i]; i++) {
		ret += (char)str[i];
	}

	return ret;
}

/* unsigned int id; */
/* 	unsigned int play_time; */
/* 	char *name; */
/* 	char *gener; */
/* 	char *location; */
/* 	char *start_point; */
/* 	char *uninstaller; */

int
gr_init(Game_rec *grp, const char *name, const char *location, const char *sp, const char *uninstaller)
{
	int tok = 0;

	if (!name || !location || !sp) {
		return -1;
	}

	grp->id        = gr_make_id(name);
	grp->play_time = 0;

	grp->name        = estrdup(name);
	grp->location    = estrdup(location);
	grp->start_point = estrdup(sp);

	if (uninstaller) {
		grp->uninstaller = estrdup(uninstaller);
	}

	//TODO not freeing but checing
	if (!grp->name) {
		tok |= 1;
	}
	if (!grp->location) {
		tok |= 2;
	}
	if (!grp->start_point) {
		tok |= 4;
	}

	//
	if (tok) {
		warn("GR INIT!");
	}

	return tok;
}

void
gr_edit(Game_rec *grp, unsigned int play_time, const char *name, const char *gener, const char *location, const char *sp, const char *unistaller)
{

}

int
gr_add(Game_rec newrec)
{
	Game_rec *grp;

	if(gr_tab.game_rec == NULL) {
		gr_tab.game_rec =
			(Game_rec *)emalloc(GR_INIT * sizeof(Game_rec));
		if (gr_tab.game_rec == NULL) {
			return -1;
		}
		gr_tab.max = GR_INIT;
		gr_tab.ngames = 0;
	} else if (gr_tab.ngames >= gr_tab.max) {
		grp = (Game_rec *)erealloc(gr_tab.game_rec,
				(GR_GROW*gr_tab.max) * sizeof(Game_rec));
		if (grp == NULL) {
			return -1;
		}
		gr_tab.max *= GR_GROW;
		gr_tab.game_rec = grp;
	}

	gr_tab.game_rec[gr_tab.ngames] = newrec;

	return gr_tab.ngames++;
}

int
gr_delete(int id)
{
	int i;

	for (i = 0; i < gr_tab.ngames; i++) {
		if (gr_tab.game_rec[i].id == id) {
			memmove(gr_tab.game_rec+i, gr_tab.game_rec+i+1,
					(gr_tab.ngames-(i+1)) * sizeof(Game_rec));
			gr_tab.ngames--;
			return 1;
		}
	}

	return 0;
}

int
gr_is_dup(Game_rec game)
{
	int i;

	for (i = 0; i < gr_tab.ngames; i++) {
		if (grcmp(gr_tab.game_rec[i], game) == 0) {
			return 1;
		}
	}

	return 0;
}

int
grcmp(Game_rec src, Game_rec dst)
{
	if (!(src.location || dst.location ||
		src.name || dst.name ||
		src.start_point || dst.start_point)) {
		return -1;
	}

	if (strcmp(src.location, dst.location) == 0 &&
			strcmp(src.name, dst.name) == 0 &&
			strcmp(src.start_point, dst.start_point) == 0) {
		return 0;
	}

	return 1;
}

Game_rec *
grcpy(Game_rec *dst, Game_rec *src)
{
	dst->id = src->id;
	dst->location = estrdup(src->location);
	dst->name = estrdup(src->name);
	dst->start_point = estrdup(src->start_point);

	return dst;
}

Game_rec *
grdup(Game_rec *gr)
{
	Game_rec *dup;

	dup = (Game_rec *)ecalloc(1, sizeof *dup + 1);
	if (!dup) {
		return NULL;
	}

	dup->location = estrdup(gr->location);
	dup->name = estrdup(gr->name);
	dup->start_point = estrdup(gr->start_point);

	return dup;
}
