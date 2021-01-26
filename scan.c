#if defined(unix) || defined(__unix__) || defined(__unix)
	#define _GNU_SOURCE
#else
	#include <libgen.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "eprintf.h"
#include "util.h"
#include "ccread.h"
#include "gamerec.h"

/* funcs */
static char **get_file_list(const char *path);
static char *scan_for(const char *location, const char *gameName, int func(const char *, const char *));
static int scan_inclusions(const char *location);
static int find_games(const char *path);

/* vars */
extern struct Gr_tab gr_tab;

static char **
get_file_list(const char *path)
{
	struct dirent **namelist;
	char **ret;
	char *buf;

	char rpath[PATH_MAX];
	int n, c, len;

	n = scandir(path, &namelist, NULL, alphasort);
	if (n == -1) {
		warn("scandir:");
		return NULL;
	}

	ret = (char **)ecalloc(n, sizeof(char **));
	if (!ret) {
		return NULL;
	}

	c = 0;
	while (n--)
	{
		if (!isDotName(namelist[n]->d_name)
				&& realpath((buf=cat_fnames(path, namelist[n]->d_name)), rpath)) {
			len = strlen(rpath) + 1;
			ret[c] = (char *)emalloc(sizeof(char *) * len);
			if (ret[c]) {
				memcpy(ret[c++], rpath, len);
			}
			free(buf);
		}

		free(namelist[n]);
	}
	free(namelist);

	ret[c] = NULL;

	return ret;
}

/* TODO add subdir search */
static char *
scan_for(const char *location, const char *gameName, int check(const char *, const char *))
{
	int i, stat, len;
	char *ret = NULL;
	char **list = NULL;

	list = get_file_list(location);

	for (i = 0; list[i]; i++)
	{
		if (!isExcludeName(list[i]))
		{
			if ((stat = isDirectory(list[i]))) {
				;
			} else if (stat == 0) {
				if (check(list[i], gameName)) {
					len = strlen(list[i]) + 1;
					ret = (char *)emalloc(len);
					if (ret) {
						memcpy(ret, list[i], len);
						break;
					}
				}
			}
		}
	}

	pp_free(list);

	return ret;
}

/* TODO */
static int
scan_inclusions(const char *location)
{

	return 0;
}

/* Finding games in all directories in "path"
 * with depth 1 */
static int
find_games(const char *path)
{
	int i;
	char *sp = NULL, *uninst = NULL, *game_name;
	Game_rec *gr;

	printf("Finding games...\n");

	char **list = get_file_list(path);
	if (!list) {
		return -1;
	}

	/* TODO make run with depth 3 */
	for (i = 0; list[i]; i++) {
		if (!isDirectory(list[i])
			|| isExcludeName(list[i])) {
			continue;
		}

		game_name = basename(list[i]);
		if (!game_name) {
			warn("basename:");
			continue;
		}
		sp = scan_for(list[i], game_name, isStartPoint);

		if (sp)
		{
			uninst = scan_for(list[i], NULL, isUninstaller);

			gr = gr_init(game_name, list[i], sp, uninst);
			if (!gr) {
				continue;
			}

			if (!gr_is_dup(*gr)) {
				gr_add(gr);
				free(gr);
			} else {
				grp_free(gr);
			}

			free(sp);
			if (uninst) free(uninst);
		}
	}

	pp_nfree(list, i);

	return 0;
}

void
check_gr_tab(struct Gr_tab tab)
{
	int i;
	struct Gr_prop prop;
	for (i = 0; i < tab.ngames; i++) {
		gr_get_props(&tab.game_rec[i], prop);
		gr_set_props(&tab.game_rec[i], &prop);
	}
}

int
scan(const char *path)
{
	char rpath[PATH_MAX];

	if (!realpath(path, rpath)) {
		warn("Aborting scan!");
		return -1;
	}

	if(!isDirectory(rpath)) {
		warn("\"%s\": its not directory! Aborting scan!", rpath);
		return -1;
	}

	/* TODO */
	scan_inclusions(NULL);

	db_read_settings();
	db_read_cached_recs();
	find_games(rpath);
	check_gr_tab(gr_tab);
	db_cache_recs();

	return 0;
}
