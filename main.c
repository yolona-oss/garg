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
char **confPathes;
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

	printf("      [II] getting file list\n"); fflush(stdout);

	n = scandir(path, &namelist, NULL, alphasort);
	if (n == -1) {
		warn("scandir:");
		return NULL;
	}

	printf("size - %d\n", n); fflush(stdout);
	ret = (char **)malloc(sizeof(char **) * (n+1));
	if (!ret) {
		warn("malloc:");
		return NULL;
	}

	int tmp = n;
	while (tmp--) {
		printf(" == %s\n", namelist[tmp]->d_name);
	}

	c = 0;
	while (n--)
	{
		if (!isDotName(namelist[n]->d_name) && getRPath(namelist[n]->d_name, path, rpath)) {
			len = strlen(rpath) + 1;
			printf("%d - len of : %s\n", len, rpath);
			printf("+"); fflush(stdout);
			if ((ret[c] = (char *)malloc(sizeof(char *) * len))) {
				memcpy(ret[c++], rpath, len);
				printf(" RET: %s\n", ret[c-1]);
			} else {
				warn("malloc:");
			}
		}

		free(namelist[n]);
	}
	free(namelist);
	printf("\n"); fflush(stdout);

	*count = c;

	printf("      [II] |%d| getted\n", c);

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

	printf("    [II] Scanning %s\n", location);

	if (!(dirs = (char **)malloc(sizeof(char **) * 10000))) {
		warn("malloc:");
		return NULL;
	}
	
	list = getFileList(location, &ecount);

	for (i = 0; i < ecount; i++)
	{
		printf("      [II] Checking file %s\n", list[i]); fflush(stdout);
		if (!isExcludeName(list[i]))
		{
			/* printf("HEAR\n"); fflush(stdout); */
			if ((stat = isDirectory(list[i]))) {
				/* printf("      [II] Adding dir to array "); fflush(stdout); */
				/* if ((dirs[i_d] = (char *)malloc(strlen(list[i]+1)))) { */
				/* 	strcpy(dirs[i_d++], list[i]); */
				/* } else { */
				/* 	warn("malloc:"); */
				/* } */
				/* printf("%s\n", dirs[i_d-1]); fflush(stdout); */
			} else if (stat == 0) {
				printf("       [@@] Checking - \"%s\"\n", list[i]); fflush(stdout);

				if (isStartPoint(list[i])) {
					printf("     [SS] Congrat!!\n"); fflush(stdout);

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

//try use nftw instead loop
int
findGames(const char *path, int id)
{
	char rpath[PATH_MAX];
	char *startPoint;

	DIR *d;
	struct dirent *dir;

	printf("Finding games...\n");

	if (!(d = opendir(path))) {
		warn("Cant open dir: %s:", path);
		return -1;
	}

	while ((dir = readdir(d)))
	{
		if (isDotName(dir->d_name) || !getRPath(dir->d_name, path, rpath)
				|| isExcludeName(rpath)) {
			continue;
		}

		if (isDirectory(rpath)) {
			printf(":: Start finding in: %s\n", rpath); fflush(stdout);
			gameName = basename(rpath);
			startPoint = searchSP(rpath);
			if (startPoint) {
				/* printf("  -- Found sp: %s\n", startPoint); fflush(stdout); */
				Game[id].id = id;
				editGameEntry(id, gameName, rpath, startPoint);
				printGameEntry(id);
				id++;
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
	/* readedGameEntries = readCache("/home/xewii/.cache/ga-org.conf"); */
	readedGameEntries = 0;
	gc = findGames(path, readedGameEntries);
	/* writeCache("/home/xewii/.cache/ga-org.conf"); */

	printf("###################SCAN END###################\n");
	for (int i = 0; i <= gc; i++) {
		/* printf("id          - %d\ngame        - %s\nlocation    - %s\nstart point - %s\n\n", i, Game[i].name, Game[i].location, Game[i].starPoint); */
		printGameEntry(i);
		fflush(stdout);
	}

	return gc;
}

void
usage(void)
{
	die("[-c|--config] <DIR>(<DIR>...)");
}

int main(int argc, char **argv)
{
	char *userConf = NULL;
	char tmp[PATH_MAX];

	argv0 = *argv;
	argv++;

	for (int i = 0; *argv && (*argv)[0] == '-' && (*argv)[1]; i++, argc--, argv++)
	{
		if ((*argv)[1] == '-') {
			*argv += 2;
		} else {
			(*argv)++;
		}
			if (strcmp(*argv, "c") == 0) {
				userConf = *(++argv);
			} else if (strcmp(*argv, "config") == 0) {
				userConf = *(++argv);
			} else if (strcmp(*argv, "h") == 0) {
				usage();
			} else if (strcmp(*argv, "help") == 0){
				usage();
			} else {
				warn("Unknown argument: \"%s\"", *argv);
				usage();
			}
	}

	int len;

	confPathes = (char **)malloc(sizeof(char **) * ((userConf) ? 3 : 2));

	len = sprintf(tmp, "%s/%s", getenv("HOME"), ".ga-org.conf") + 1;
	confPathes[0] = (char *)malloc(len);
	memcpy(confPathes[0], tmp, len);

	len = sprintf(tmp, "%s/%s", getenv("HOME"), ".config/ga-org.conf") + 1;
	confPathes[1] = (char *)malloc(len);
	memcpy(confPathes[1], tmp, len);

	if (userConf) {
		len = strlen(userConf) + 1;
		confPathes[2] = (char *)malloc(len);
		memcpy(confPathes[2], userConf, len);
	}

	//add multi path option
	scan((*argv) ? *argv : ".");

	freeSG(Game);
	freePP(exceptionName, getLenOfPP(exceptionName));
	freePP(confPathes, getLenOfPP(confPathes));

	return 0;
}
