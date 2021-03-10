#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "../utils/util.h"
#include "../utils/eprintf.h"
#include "gamerec.h"

/* vars */
extern game_tab_t gr_tab;

pid_t g_game_pid;

/* funcs */
static unsigned int gr_make_id(const char *);

static void gr_set_props(game_t *grp, game_prop_t prop);
static game_prop_t gr_get_props(game_t *grp);

int
run_game(int id)
{
	char *path = gr_tab.game_rec[id].start_point;
	char run[strlen(path) + 3];

	esnprintf(run, sizeof(run), "/.%s", path);

	switch (g_game_pid = fork()) {
		case 0:
		setsid();

		/* int devnull = open("/dev/devnull", O_WRONLY); */
		/* if (!devnull) { */
		/* 	/1* status text handler TODO *1/ */
		/* 	break; */
		/* } */
		/* dup2(devnull, 1); */
		/* dup2(devnull, 2); */

		char *argv[4];
		argv[0] = basename(run);
		argv[1] = NULL;

		execvp(run, argv);

		exit(EXIT_SUCCESS);
		break;

		default:
		;
		int status = 0;
		pid_t wpid;

		do {
			wpid = waitpid(g_game_pid, &status, WUNTRACED);

			if (wpid == -1) {
				warn("waitpid: ");
				// show tui message
				break;
			}

			if (WIFEXITED(status)) {
				if (WEXITSTATUS(status)) {
					// show tui message
				}
			} else {
				//TODO
			}
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));

		break;

		case -1:
		//log error TODO
		exit(EXIT_FAILURE);
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

void
gr_print(game_t *Game)
{
	if (Game) {
		printf("\n##########################################\n");
		printf(" id          - %d\n", Game->id);
		printf(" play time   - %d\n", Game->play_time.tm_min);
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
gr_get_props(game_t *grp)
{
	game_prop_t prop;
	if (!grp) {
		return prop;
	} 

	if (!isExist(grp->location)) {
		prop.location = 1;
	} else {
		prop.location = 0;
	}

	if (!isStartPoint(grp->start_point, grp->name)) {
		prop.start_point = 1;
	} else {
		prop.start_point = 0;
	}

	if (grp->uninstaller && !isUninstaller(grp->uninstaller, NULL)) {
		prop.uninstaller = 1;
	} else {
		prop.uninstaller = 0;
	}
	
	if (grp->icon && !isExist(grp->icon)) {
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

	return ret % UINT_MAX;
}

game_t *
gr_init(const char *name, const char *location, const char *sp, const char *uninst)
{
	if (!name || !location || !sp) {
		return NULL;
	}

	game_t *grp = (game_t *)ecalloc(1, sizeof*grp);

	grp->id        = gr_make_id(name);

	grp->play_time.tm_min = 0;
	grp->last_time.tm_sec = -1;

	grp->name        = estrdup(name);
	grp->location    = estrdup(location);
	grp->start_point = estrdup(sp);

	if (uninst) {
		grp->uninstaller = estrdup(uninst);
	} else {
		grp->uninstaller = NULL;
	}

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
gr_edit(game_t *grp, unsigned int play_time, const char *name, const char *gener, const char *location, const char *sp, const char *unistaller)
{

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
gr_id(game_t *gr)
{
	return gr->id;
}

int
grcmp(game_t src, game_t dst)
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
	if (gr->uninstaller) {
		dup->uninstaller = estrdup(gr->uninstaller);
	}
	if (gr->gener) {
		dup->gener = estrdup(gr->gener);
	}

	return dup;
}
