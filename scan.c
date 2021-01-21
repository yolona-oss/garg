#if defined(unix) || defined(__unix__) || defined(__unix)
	#define _GNU_SOURCE
#else
	#include <libgen.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "main.h"
#include "eprintf.h"
#include "util.h"
#include "ccread.h"
#include "gamerec.h"

/* funcs */
static char **get_file_list(const char *path);
static char *scan_for_sp(const char *location, const char *gameName);
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
scan_for_sp(const char *location, const char *gameName)
{
	int i, stat, len;
	char *sp = NULL;
	char **list = NULL;

	/* char **dirs = NULL; */
	/* dirs = (char **)emalloc(sizeof(char **) * 100); */
	/* if (!dirs) { */
	/* 	return NULL; */
	/* } */
	
	list = get_file_list(location);

	for (i = 0; list[i]; i++)
	{
		if (!isExcludeName(list[i]))
		{
			if ((stat = isDirectory(list[i]))) {
				/* if ((dirs[i_d] = (char *)malloc(strlen(list[i]+1)))) { */
				/* 	strcpy(dirs[i_d++], list[i]); */
				/* } */
			} else if (stat == 0) {
				if (isStartPoint(list[i], gameName)) {
					len = strlen(list[i]) + 1;
					sp = (char *)emalloc(len);
					if (sp) {
						memcpy(sp, list[i], len);
					}
					break;
				}
			}
		}
	}

	pp_free(list, i);
	/* pp_free(dirs); */

	return sp;
}

/* TODO */
static int
scan_inclusions(const char *location)
{

	return 0;
}

unsigned int
str_to_int(const char *str)
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

/* Finding games in all directories in "path"
 * with depth 1 */
static int
find_games(const char *path)
{
	int i;
	char *start_point, *game_name;
	/* Game_rec *gr; */
	Game_rec gr;

	printf("Finding games...\n"); fflush(stdout);

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
		start_point = scan_for_sp(list[i], game_name);

		if (start_point)
		{
			/* gr_init(gr, game_name, list[i], start_point, NULL); */
			gr.location = estrdup(list[i]);
			gr.name = estrdup(game_name);
			gr.start_point = estrdup(start_point);
			gr.id = str_to_int(game_name);

			if (!gr_is_dup(gr)) {
				if (gr_add(gr) < 0) {
					warn("Cant add game!");
					free(gr.location);
					free(gr.name);
					free(gr.start_point);
				}
			} else {
				free(gr.location);
				free(gr.name);
				free(gr.start_point);
			}

			free(start_point);
		}
	}

	pp_free(list, i);

	return 0;
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

	readConfig();
	readCache();
	find_games(rpath);
	writeCache();

	return 0;
}
