#if defined(unix) || defined(__unix__) || defined(__unix)
	#define _GNU_SOURCE
#else
	#include <libgen.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "../main.h"
#include "scan.h"
#include "../utils/eprintf.h"
#include "../utils/util.h"
#include "../games/gamerec.h"
#include "../db/dbman.h"

/* funcs */
/* static int special_id(const char *name); */

static char **get_file_list(const char *path);
static char *scan_for(const char *location, const char *game_name, int maxdepth, 
		int func(const char *, const char *));
static int find_games(const char *path);

/* vars */
extern struct Gr_tab gr_tab;
extern int g_scan_depth;

/* const spec_name_t special_names[C_SPEC] = { */ 
/* 	{ "Steam", scan_steam }, */
/* }; */

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

/* /1* returns id of specials name *1/ */
/* /1* or -1 if its not special *1/ */
/* static int */ 
/* special_id(const char *name) */
/* { */
/* 	int i; */
/* 	for (i = 0; special_names[i].val; i++) { */
/* 		if (strcmp(special_names[i].val, name) == 0) { */
/* 			return i; */
/* 		} */
/* 	} */

/* 	return -1; */
/* } */

static char *
scan_for(const char *location, const char *game_name, int maxdepth,
		int check(const char *, const char *))
{
	if (!maxdepth) {
		return 0;
	}

	int i, stat, len;
	char *ret = NULL;
	char **list = NULL;

	list = get_file_list(location);

	for (i = 0; list[i]; i++)
	{
		if (!isExcludeName(list[i]))
		{
			len = strlen(list[i]) + 1;
			if ((stat = isDirectory(list[i]))) {
			} else if (stat == 0) {
				if (check(list[i], game_name)) {
					ret = (char *)emalloc(len);
					if (ret) {
						memcpy(ret, list[i], len);
						break;
					}
				}
			}
		}
	}

	if (!ret && maxdepth > 0) {
		
		if (!ret) {
			char **dirl = pp_sort(list, isDirectory);
			if (dirl) {
				for (i = 0; dirl[i]; i++) {
					scan_for(dirl[i], game_name, --maxdepth, check);
				}
				free(dirl);
			}
		}
	}

	pp_free(list);

	return ret;
}

int
scan_inclusions(const char *location)
{

	return 0;
}

/* Finding games in all directories in "path"
 * with depth 1.
 * Detecting special file names and redirecting 
 * search algorithm to another. */
static int
find_games(const char *path)
{
	int i;
	char *sp = NULL, *uninst = NULL, *game_name;
	game_t *gr;

	/* char *(*scanner)(const char *, const char *, int, int func(const char *, const char *)) = NULL; */

	/* printf("Finding games...\n"); */

	char **list = get_file_list(path);
	if (!list) {
		return -1;
	}

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

		/* Scanning Steam dir */
		/* TODO make its run as function */
		if (strcmp(game_name, "Steam") == 0) {
			char *common = cat_fnames(list[i], "steamapps/common");
			find_games(common);
			free(common);
			continue;
		}

		sp = scan_for(list[i], game_name, g_scan_depth, isStartPoint);

		if (sp)
		{
			uninst = scan_for(list[i], NULL, g_scan_depth, isUninstaller);

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
update(void)
{
	check_gr_tab(gr_tab);
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

	find_games(rpath);
	db_cache_recs();

	return 0;
}
