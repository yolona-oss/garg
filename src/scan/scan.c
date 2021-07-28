#if defined(unix) || defined(__unix__) || defined(__unix)
	#define _GNU_SOURCE
#else
	#include <libgen.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* #include "ftw.h" */
/* #include "fnmatch.h" */
#include "fts.h"

#include "../main.h"
#include "scan.h"
#include "../utils/eprintf.h"
#include "../utils/util.h"
#include "../games/gamerec.h"
#include "../db/dbman.h"

/* vars */
extern struct Gr_tab gr_tab;
extern int g_scan_depth;

int
scan_inclusions(const char *location)
{

	return 0;
}

/* sp = scan_for(list[i], game_name, g_scan_depth, isStartPoint); */

/* if (sp) */
/* { */
/* 	icon = scan_for(list[i], NULL, g_scan_depth, isIcon); */
/* 	uninst = scan_for(list[i], NULL, g_scan_depth, isUninstaller); */

/* 	gr = gr_init(game_name, list[i], sp, uninst, icon); */
/* 	if (!gr) { */
/* 		continue; */
/* 	} */

/* 	if (!gr_is_dup(*gr)) { */
/* 		gr_add(gr); */
/* 		free(gr); */
/* 	} else { */
/* 		grp_free(gr); */
/* 	} */

int
scan(const char *path)
{
	char *rpath = (char *)emalloc(sizeof(char *) * (PATH_MAX+1));

	/* path chekcs */
	if (!realpath(path, rpath)) {
		warn("Aborting scan!");
		return -1;
	}

	if(!isDirectory(rpath)) {
		warn("\"%s\": its not directory! Aborting scan!", rpath);
		return -1;
	}

	char *start_point,
		 *uninstaller,
		 *icon;
	char **game_paths, **paths;

	FTSENT *node, *game_node;
	FTS *game_ftree, *ftree;

	game_t *gr;

	game_paths = (char **)emalloc(sizeof(char **) * 2);
	paths = (char **)emalloc(sizeof(char **) * 2);
	game_paths[1] = NULL;
	paths[0] = rpath;
	paths[1] = NULL;

	ftree = fts_open(paths, FTS_NOCHDIR, 0);
	if (!ftree) {
		warn("fts_open:");
		return -1;
	}

	while ((node = fts_read(ftree)))
	{
		if (node->fts_level > 0 && node->fts_name[0] == '.') {
			fts_set(ftree, node, FTS_SKIP);
		} else if (strcmp(node->fts_path, paths[0]) &&
				node->fts_info & FTS_D &&
				node->fts_level < 2)
			/* going for each dir in rpath in first level */
		{
			game_paths[0] = node->fts_path;
			game_ftree = fts_open(game_paths, FTS_NOCHDIR, 0);
			if (!game_ftree) {
				warn("fts_open:");
				return -1;
			}

			start_point = uninstaller = icon = NULL;
			while ((game_node = fts_read(game_ftree)))
			/* searching for game properties */
			{
				if (game_node->fts_name[0] == '.') {
					fts_set(game_ftree, game_node, FTS_SKIP);
				}
				else if (game_node->fts_info & FTS_F &&
						game_node->fts_level < g_scan_depth)
				/* cheking if cur file is an game property */
				{
					if (!start_point && isStartPoint(game_node->fts_path, node->fts_name)) {
						start_point = estrdup(game_node->fts_path);
					}
					if (!uninstaller && isUninstaller(game_node->fts_path, "")) {
						uninstaller = estrdup(game_node->fts_path);
					}
					if (!icon && isIcon(game_node->fts_path, "")) {
						icon = estrdup(game_node->fts_path);
					}
				}
			}
			
			gr = gr_init(node->fts_name, node->fts_path, start_point, uninstaller, icon);
			if (!gr) continue;
			if (!gr_is_dup(*gr)) {
				gr_add(gr);
				free(gr);
			} else {
				grp_free(gr);
			}

			if (start_point) free(start_point);
			if (uninstaller) free(uninstaller);
			if (icon) free(icon);
			if (fts_close(game_ftree)) {
				warn("fts_close:");
				return -1;
			}
		}
	}

	if (fts_close(ftree)) {
		warn("fts_close:");
		return -1;
	}

	free(game_paths);
	free(paths);
	free(rpath);

	db_cache_recs();

	return 0;
}
