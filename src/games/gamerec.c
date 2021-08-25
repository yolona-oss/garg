#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>
#include <assert.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <sqlite3.h>

#include "../utils/util.h"
#include "../utils/eprintf.h"
#include "../db/dbman.h"
#include "gamerec.h"

/* vars */
extern game_tab_t gr_tab;
pid_t g_game_pid;

/* funcs */
static unsigned int gr_make_id(const char *);
static void gr_set_props(game_t *grp, game_prop_t prop);
static game_prop_t gr_get_props(game_t *grp);

/* run some game with id 
 * returns:
 * 		1  - game is running
 * 		0  - child stoped
 * 		-1 - some error */
int
run_game(int id)
{
	game_t *gr = grt_find(id);
	char *path = gr->start_point;
	char run[strlen(path) + 3];

	esnprintf(run, sizeof(run), "/.%s", path);
	time_t cur_time = time(NULL);
	time_t end_time = -1;

	/* last time setup */
	gr->last_time = time(NULL);

	g_game_pid = fork();
	switch (g_game_pid) {
		case 0: //child
		{
			/* setsid(); */

			/* int devnull = open(, O_CREAT|O_WRONLY|O_TRUNC, /1* open or create and open with 600 mask *1/ */
			/* S_IRWXU); */
			/* if (!devnull) { */
			/* 	/1* status text handler TODO *1/ */
			/* 	break; */
			/* } */
			/* dup2(devnull, 1); */
			/* dup2(devnull, 2); */

			/* char *argv[2] = { run, gr->start_argv, NULL}; */
			char *argv[] = { basename(run), NULL};

			execv(run, argv);
			return 1;
		}
		break;

		default: //parent
		{
			int status = 0;
			pid_t wpid;

			do {
				wpid = waitpid(g_game_pid, &status, WUNTRACED);

				if (wpid == -1) {
					warn("waitpid: ");
					break;
				}

				if (WIFEXITED(status)) {
					if (WEXITSTATUS(status)) {
					}
				} else {
				}

			} while (!WIFEXITED(status) && !WIFSIGNALED(status));

			end_time = time(NULL);
			time_t diff = difftime(end_time, cur_time); /* with cpu word size eq 64 you can play(with recirding) only 292,471,208,677.5 year :__D */
			gr_play_time_append(gr, diff);
		}
		break;

		case -1:
			//log error TODO
			/* exit(EXIT_FAILURE); */
			return -1;
			break;
	}

	return 0;
}

void
grp_free(game_t *grp)
{
	free(grp->name);
	free(grp->location);
	free(grp->start_point);

	if (grp->icon)
		free(grp->icon);
	if (grp->gener)
		free(grp->gener);
	if (grp->start_argv)
		free(grp->start_argv);
	if (grp->uninstaller)
		free(grp->uninstaller);

	free(grp);
}

void
grp_check_free(game_t *grp)
{
	if (grp) {
		if (grp->location)
			free(grp->location);
		if (grp->start_point)
			free(grp->start_point);
		if (grp->name)
			free(grp->name);
		if (grp->icon)
			free(grp->icon);
		if (grp->gener)
			free(grp->gener);
		if (grp->start_argv)
			free(grp->start_argv);
		if (grp->uninstaller)
			free(grp->uninstaller);
		free(grp);
	}
}

void
grt_free(game_tab_t grt)
{
	for (int i = 0; i < gr_tab.ngames; i++) {
		free(gr_tab.game_rec[i].name);
		free(gr_tab.game_rec[i].location);
		free(gr_tab.game_rec[i].start_point);

		if (gr_tab.game_rec[i].start_argv)
			free(gr_tab.game_rec[i].start_argv);
		if (gr_tab.game_rec[i].gener)
			free(gr_tab.game_rec[i].gener);
		if (gr_tab.game_rec[i].icon)
			free(gr_tab.game_rec[i].icon);
		if (gr_tab.game_rec[i].uninstaller)
			free(gr_tab.game_rec[i].uninstaller);
	}
	free(gr_tab.game_rec);
}

/* TODO */
void
gr_print(game_t *Game)
{
	if (Game) {
		printf("\n##########################################\n");
		printf(" id          - %d\n", Game->id);
		/* printf(" play time   - %d\n", Game->play_time); */
		printf(" name        - %s\n", Game->name);
		printf(" icon        - %s\n", Game->icon);
		printf(" gener       - %s\n", Game->gener);
		printf(" start opts  - %s\n", Game->start_argv);
		printf(" location    - %s\n", Game->location);
		printf(" start point - %s\n", Game->start_point);
		printf(" uninstaller - %s\n", Game->uninstaller);
		printf("##########################################\n");

		fflush(stdout);
	} else {
		warn("Cant print game record: record dose not exist");
	}
}

static game_prop_t
gr_get_props(game_t *gr)
{
	game_prop_t prop;
	assert(gr);

	if (!isExist(gr->location)) {
		prop.location = 1;
	} else {
		prop.location = 0;
	}

	if (!isStartPoint(gr->start_point, gr->name)) {
		prop.start_point = 1;
	} else {
		prop.start_point = 0;
	}

	if (gr->uninstaller && !isUninstaller(gr->uninstaller, NULL)) {
		prop.uninstaller = 1;
	} else {
		prop.uninstaller = 0;
	}
	
	if (gr->icon && !isExist(gr->icon)) {
		prop.icon = 1;
	} else {
		prop.icon = 0;
	}

	return prop;
}

static void
gr_set_props(game_t *grp, game_prop_t prop)
{
	grp->properties.icon = prop.icon;
	grp->properties.location = prop.location;
	grp->properties.start_point = prop.start_point;
	grp->properties.uninstaller = prop.uninstaller;
}

void
check_gr_tab()
{
	game_prop_t prop;
	for (int i = 0; i < gr_tab.ngames; i++) {
		prop = gr_get_props(&gr_tab.game_rec[i]);
		gr_set_props(&gr_tab.game_rec[i], prop);
	}
}

static unsigned int
gr_make_id(const char *str)
{
	unsigned int ret = 0;
	unsigned char *p;

	ret = 0;
	for (p = (unsigned char*)str; *p != '\0'; p++) {
		ret = 2 * ret + *p;
	}

	/* TODO */
	ret = ret % USHRT_MAX;
	while (grt_find(ret)) {
		ret++;
	}
	
	return ret;
}

game_t *
gr_init(const char *name, const char *location, const char *sp, const char *uninst, const char *icon)
{
	if (!name || !location || !sp) {
		return NULL;
	}

	game_t *grp = (game_t *)ecalloc(1, sizeof*grp);

	grp->id        = gr_make_id(name);

	grp->play_time = 0;
	grp->last_time = -1;

	grp->name        = estrdup(name);
	grp->location    = estrdup(location);
	grp->start_point = estrdup(sp);

	grp->uninstaller = (uninst) ? estrdup(uninst) : NULL;
	grp->icon = (icon) ? estrdup(icon) : NULL;

	char *gener = "common";
	grp->gener = estrdup(gener);

	if (!grp->location
			|| !grp->start_point
			|| !grp->name) {
		grp_check_free(grp);
		warn("cant GR INIT!");
		return NULL;
	}
	
	game_prop_t prop;
	prop = gr_get_props(grp);
	gr_set_props(grp, prop);

	return grp;
}

void
gr_edit(game_t *grp, game_t *dst)
{

}

int
gr_save(game_t *gr)
{
	sqlite3 *db = db_init();
	db_upd_rec(db, gr);
	db_close(db);

	return 0;
}

int
gr_add(game_t *newrec)
{
	game_t *grp;

	if(gr_tab.game_rec == NULL) {
		gr_tab.game_rec =
			(game_t *)emalloc(GR_INIT * sizeof(game_t));
		if (gr_tab.game_rec == NULL) {
			return -1;
		}
		gr_tab.max = GR_INIT;
		gr_tab.ngames = 0;
	} else if (gr_tab.ngames >= gr_tab.max) {
		grp = (game_t *)erealloc(gr_tab.game_rec,
				(GR_GROW*gr_tab.max) * sizeof(game_t));
		if (grp == NULL) {
			return -1;
		}
		gr_tab.max *= GR_GROW;
		gr_tab.game_rec = grp;
	}

	gr_tab.game_rec[gr_tab.ngames] = *newrec;

	return gr_tab.ngames++;
}

int
gr_delete(int id)
{
	int i;

	for (i = 0; i < gr_tab.ngames; i++) {
		if (gr_tab.game_rec[i].id == id) {
			memmove(gr_tab.game_rec+i, gr_tab.game_rec+i+1,
					(gr_tab.ngames-(i+1)) * sizeof(game_t));
			gr_tab.ngames--;
			return 1;
		}
	}

	return 0;
}

int
gr_is_dup(game_t game)
{
	int i;

	for (i = 0; i < gr_tab.ngames; i++) {
		if (grcmp(gr_tab.game_rec[i], game) == 0) {
			return 1;
		}
	}

	return 0;
}

game_t *
grt_find(int id)
{
	int i;
	for (i = 0; i < gr_tab.ngames; i++) {
		if (gr_tab.game_rec[i].id == id) {
			return &gr_tab.game_rec[i];
		}
	}

	return NULL;
}

int
grcmp(game_t src, game_t dst)
{
	if (!(src.location || dst.location ||
		src.name || dst.name ||
		src.start_point || dst.start_point)) {
		return -1;
	}

	if (src.id == dst.id || (
				strcmp(src.location, dst.location) == 0 &&
				strcmp(src.name, dst.name) == 0 &&
				strcmp(src.start_point, dst.start_point) == 0)) {
		return 0;
	}

	return 1;
}

/* TODO */
game_t *
grcpy(game_t *dst, game_t *src)
{
	dst->id = src->id;
	dst->play_time = src->play_time;
	dst->location = estrdup(src->location);
	dst->name = estrdup(src->name);
	dst->start_point = estrdup(src->start_point);
	if (src->uninstaller) {
		dst->uninstaller = estrdup(src->uninstaller);
	}
	if (src->gener) {
		dst->gener = estrdup(src->gener);
	}

	return dst;
}

/* TODO */
game_t *
grdup(game_t *gr)
{
	game_t *dup;

	dup = (game_t *)ecalloc(1, sizeof *dup + 1);
	if (!dup) {
		return NULL;
	}

	dup->id = gr->id;
	dup->play_time = gr->play_time;
	dup->location = estrdup(gr->location);
	dup->name = estrdup(gr->name);
	dup->start_point = estrdup(gr->start_point);
	dup->icon = gr->icon ? estrdup(gr->icon) : NULL;
	dup->uninstaller = gr->uninstaller ? estrdup(gr->uninstaller) : NULL;
	dup->gener = gr->gener ? estrdup(gr->gener) : NULL;

	return dup;
}

const char *
gr_play_time_human(game_t *gr)
{
	time_t min = gr->play_time/60;
	if (min < 120) {
		return bprintf("%ld mins", min);
	} else {
		return bprintf("%ld hours and %ld mins", min/60,
												 min - (min/60)*60); /* INTEGERS :)) */
	}
}

int
gr_play_time_append(game_t *gr, time_t time)
{
	/* TODO add chekc max val ))) */
	gr->play_time += time;
	return 0;
}

const char *
gr_last_time_human(game_t *gr)
{
	char *last_time = gr->last_time ? ctime(&gr->last_time) : "";
	if (last_time[0] != '\0') str_del_last_sym(last_time);

	return last_time;
}
