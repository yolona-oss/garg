#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <libgen.h>

#include "scan.h"
#include "global.h"
#include "util.h"
#include "ccread.h"

struct Gr_tab gr_tab;

char **
getFileList(const char *path, int *count)
{
	struct dirent **namelist;
	char **ret;

	char rpath[PATH_MAX];
	int n = 0, c, len;

	n = scandir(path, &namelist, NULL, alphasort);
	if (n == -1) {
		warn("scandir:");
		return NULL;
	}

	ret = (char **)malloc(sizeof(char **) * (n+1));
	if (!ret) {
		warn("malloc:");
		return NULL;
	}

	c = 0;
	while (n--)
	{
		if (!isDotName(namelist[n]->d_name)
				&& getRPath(namelist[n]->d_name, path, rpath)) {
			len = strlen(rpath) + 1;
			if ((ret[c] = (char *)malloc(sizeof(char *) * len))) {
				memcpy(ret[c++], rpath, len);
			} else {
				warn("malloc:");
			}
		}

		free(namelist[n]);
	}
	free(namelist);

	*count = c;

	return ret;
}

char *
searchSP(const char *location, const char *gameName)
{
	int ecount, i, stat, len;
	char *sp = NULL;
	char **list;

	char **dirs = NULL;

	int i_d;
	i_d = 0;

	allerMsg("    [II] Scanning %s\n", location);

	if (!(dirs = (char **)malloc(sizeof(char **) * 10000))) {
		warn("malloc:");
		return NULL;
	}
	
	list = getFileList(location, &ecount);

	for (i = 0; i < ecount; i++)
	{
		allerMsg("      [II] Checking file %s\n", list[i]);
		if (!isExcludeName(list[i]))
		{
			if ((stat = isDirectory(list[i]))) {
				/* if ((dirs[i_d] = (char *)malloc(strlen(list[i]+1)))) { */
				/* 	strcpy(dirs[i_d++], list[i]); */
				/* } */
			} else if (stat == 0) {
				if (isStartPoint(list[i], gameName)) {
					len = strlen(list[i]) + 1;
					if ((sp = (char *)malloc(len))) {
						memcpy(sp, list[i], len);
					} else {
						warn("malloc:");
					}
					break;
				}
			}
		}
	}

	freePP(list, ecount);
	freePP(dirs, i_d);

	/* if (!sp) { */
	/* 	for (i = 0; i < i_d; i++) { */
	/* 		sp = searchSP(dirs[i]); */
	/* 	} */
	/* } */

	return sp;
}

//try use nftw instead loop
int
findGames(const char *path, struct Game_rec **Games)
{
	int curID, globID;
	char rpath[PATH_MAX];
	char *startPoint, *gameName;

	DIR *d;
	struct dirent *dir;

	struct Game_rec *bufGame;

	printf("Finding games...\n");

	if (!(d = opendir(path))) {
		warn("Cant open dir: %s:", path);
		return -1;
	}

	globID = countGameEntries(Games);

	curID = 0;
	while ((dir = readdir(d)))
	{
		if (isDotName(dir->d_name)
				|| !getRPath(dir->d_name, path, rpath)
				|| isExcludeName(rpath)) {
			continue;
		}

		if (isDirectory(rpath)) {
			gameName = basename(rpath);

			/* need check for existence in cache */
			startPoint = searchSP(rpath, gameName);

			if (startPoint) {
				bufGame = (struct Game_rec *)calloc(1, sizeof(struct Game_rec));
				addGameEntry(bufGame, gameName, rpath, startPoint);
				if (bufGame && isGameEntryUniq(Games, bufGame)) {
					Games[globID] = (struct Game_rec *)calloc(1, sizeof(struct Game_rec));
					if ((Games[globID]) &&
								addGameEntry(Games[globID], gameName, rpath, startPoint) == 3) {
							globID++;
							curID++;
					} else {
						warn("Cant add game entry");
					}
				}

				if (bufGame)
					freePSG(bufGame);
				free(startPoint);
			}
		}
	}

	if (closedir(d) == -1) {
		warn("Cant close dir: %s:", path);
		return -1;
	}

	return globID - 1;
}


int
scan(const char *path, struct Game_rec **Games)
{
	char rpath[PATH_MAX];

	if (!getRPath("", path, rpath)) {
		return -1;
	}

	if(!isDirectory(rpath)) {
		warn("\"%s\": its not directory", rpath);
		return -1;
	}

	readConfig();
	/* readCache(Games); */
	findGames(path, Games);

	/* trimCache("/home/xewii/.cache/ga-org.conf"); */
	/* writeCache(Games); */

	return 0;
}


