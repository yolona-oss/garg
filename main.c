#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <libgen.h>

#include "main.h"
#include "global.h"
#include "util.h"
#include "ccread.h"

struct game Game[MAX_GAMES+1];
char *userConf;
char **exceptionName;
/* char *exceptionPath[MAX_EXCEPTIONS]; */
/* char *inclusions[MAX_INCLUSIONS]; */

char **
getFileList(const char *path, int *count)
{
	struct dirent **namelist;
	char **ret;

	char rpath[PATH_MAX];
	int n = 0, c, len;

	allerMsg("      [II] getting file list\n");

	n = scandir(path, &namelist, NULL, alphasort);
	if (n == -1) {
		warn("scandir:");
		return NULL;
	}

	allerMsg("    [!!] nl size - %d\n", n);
	ret = (char **)malloc(sizeof(char **) * (n+1));
	if (!ret) {
		warn("malloc:");
		return NULL;
	}

	c = 0;
	while (n--)
	{
		if (!isDotName(namelist[n]->d_name) && getRPath(namelist[n]->d_name, path, rpath)) {
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

	allerMsg("      [II] %d getted\n", c);

	return ret;
}

char *
searchSP(const char *location)
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
				/* allerMsg("      [II] Adding dir to array "); */
				/* if ((dirs[i_d] = (char *)malloc(strlen(list[i]+1)))) { */
				/* 	strcpy(dirs[i_d++], list[i]); */
				/* } else { */
				/* 	warn("malloc:"); */
				/* } */
				/* allerMsg("%s\n", dirs[i_d-1]); */
			} else if (stat == 0) {
				allerMsg("       [@@] Checking - \"%s\"\n", list[i]);

				if (isStartPoint(list[i])) {
					allerMsg("     [SS] Congrat!!\n");

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
	/* 	printf("    [II] Start searching in Subdir\n"); */
	/* 	for (i = 0; i < i_d; i++) { */
	/* 		sp = searchSP(dirs[i]); */
	/* 	} */
	/* } */

	/* printf("   [II] End scanning\n"); */

	return sp;
}

#include "ccread.h"

//try use nftw instead loop
int
findGames(const char *path, int id)
{
	int curID;
	char rpath[PATH_MAX];
	char *startPoint;

	DIR *d;
	struct dirent *dir;

	printf("Finding games...\n");

	if (!(d = opendir(path))) {
		warn("Cant open dir: %s:", path);
		return -1;
	}

	curID = 0;
	int uniq;
	while ((dir = readdir(d)))
	{
		uniq = 1;
		if (isDotName(dir->d_name) || !getRPath(dir->d_name, path, rpath)
				|| isExcludeName(rpath)) {
			continue;
		}

		if (isDirectory(rpath)) {
			allerMsg(":: Start finding in: %s\n", rpath);
			gameName = basename(rpath);
			/* ************************************************* */
			/* LOOOOOOOOOOOOK AT MEEEEEEEEEEEEEEEEEE (TODO) */
			/* ************************************************* */
			for (int f = 0; f < id-curID; f++) {
				if (strcmp(gameName, Game[f].name) == 0) {
					uniq=1;
					break;
				}
			}
			if (!uniq)
				continue;

			startPoint = searchSP(rpath);
			if (startPoint) {
				allerMsg("  -- Found sp: %s\n", startPoint);

				Game[id].id = id;
				editGameEntry(id, gameName, rpath, startPoint);

				id++;
				curID++;
			}

			free(startPoint);
		}
	}

	if (closedir(d) == -1) {
		warn("Cant close dir: %s:", path);
		return -1;
	}

	return id - 1;
}

int
scan(const char *path)
{
	int gc, readedGameEntries;
	char rpath[PATH_MAX];

	if (!getRPath("", path, rpath)) {
		return -1;
	}

	if(!isDirectory(rpath)) {
		warn("\"%s\": its not directory", rpath);
		return -1;
	}

	readConfig();
	readedGameEntries = readCache();
	gc = findGames(path, (readedGameEntries > 0) ? readedGameEntries : 0) + 1;
	if (gc > 0) {
		writeCache();
	}

	/* for (int i = 0; i < gc; i++) { */
	/* 	printGameEntry(i); */
	/* } */

	return gc;
}

void
usage(void)
{
	die("[-q|--quiet] [-c|--config] <DIR>(<DIR>...)");
}

int
main(int argc, char **argv)
{
	argv0 = *argv;
	argv++;

	qflag = 0;
	dflag = 0;

	for (int i = 0; *argv && (*argv)[0] == '-' && (*argv)[1]; i++, argc--, argv++)
	{
		//modifycate
		if ((*argv)[1] == '-') {
			*argv += 2;
		} else {
			(*argv)++;
		}

		if (strcmp(*argv, "c") == 0 ||
				strcmp(*argv, "config") == 0)
		{
			int len = strlen(*argv) + 1;
			userConf = (char *)malloc(len);
			memcpy(userConf, *(++argv), len);
		}
		else if (strcmp(*argv, "q") == 0 ||
				strcmp(*argv, "quiet") == 0) {
			qflag = 1;
		}
		if (strcmp(*argv, "d") == 0 ||
				strcmp(*argv, "debug") == 0) {
			dflag = 1;
		} else if (strcmp(*argv, "h") == 0 ||
				strcmp(*argv, "help") == 0) {
			usage();
		} else {
			warn("Unknown argument: \"%s\"", *argv);
			usage();
		}
	}
	
	//add multi path option
	scan((*argv) ? *argv : ".");

	freeSG(Game);
	freePP(exceptionName, getLenOfPP(exceptionName));

	printf("EXIT\n");
	return 0;
}
