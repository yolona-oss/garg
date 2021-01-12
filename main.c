#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <dirent.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/stat.h>

#include "main.h"
#include "util.h"

struct game {
	long int id;
	char name[MAX_GAME_NAME];
	char starPoint[PATH_MAX];
} Games[MAX_GAMES];

static char gameName[MAX_FILE_NAME];
static char gameStartPoint[MAX_GAMES][PATH_MAX];
static char confPathes[3][MAX_FILE_NAME];
static char excludes[MAX_EXCLUDES][MAX_FILE_NAME];

int
isExist(char *path)
{
	if (access(path, F_OK ) == 0) {
		return 1;
	}

	return 0;
}

int
isDirectory(char *path)
{
	struct stat fileStat;

	if (stat(path, &fileStat) == -1) {
		warn("Cant get stats of file: %s", path);
		return 0;
	}

	if (S_ISDIR(fileStat.st_mode)) {
		return 1;
	}

	return 0;
}

int
isOtherDirectory(char *root, char *path)
{
	char *fileName;

	fileName = basename(path);

	if (!isDirectory(path)) {
		return 0;
	}

	if (strcmp(fileName, ".") == 0 || strcmp(fileName, "..") == 0 ||
			strcmp(root, path) == 0) {
		return 0;
	}

	return 1;
}

int
isDotName(char *name)
{
	if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
		return 1;
	}

	return 0;
}

int
isExecuteble(char *path)
{
	if (access(path, X_OK) == 0) {
		return 1;
	}

	return 0;
}

int
isExcludeName(char *name)
{
	int i;

	for (i = 0; excludes[i][0]; i++) {
		if (strcasecmp(name, excludes[i]) == 0) {
			return 1;
		}
	}

	return 0;
}

void
setGameName(char *root, int depth)
{
	char rpath[PATH_MAX];

	getRPath("", root, rpath);
	if ((getPathDepth(rpath) - depth) == 1) {
		sprintf(gameName, "%s", basename(rpath));
	}
}

int
getRPath(char *fileName, char *root, char *rpath)
{
	char path[PATH_MAX];

	if ((snprintf(path, sizeof(char) * PATH_MAX,
			"%s/%s", root, fileName)) < 1) {
		warn("Cant fill path:");
		return -1;
	}

	if (realpath(path, rpath) == NULL) {
		warn("realpath:");
		return -1;
	}

	return 0;
}

int
getConfigIndex(void)
{
	int n = -1;
	int i;

	for (i = 2; i > -1; i--) {
		if (isExist(confPathes[i])) {
			n = i;
			break;
		}
	}

	return n;
}

/* add default excludes */
int
readExcludes()
{
	int n, i;
	FILE *fd = NULL;
	char exclude[MAX_FILE_NAME];

	if ((n = getConfigIndex()) != -1)
	{
		if ((fd = fopen(confPathes[n], "r")) == NULL) {
			warn("Cant open file: %s", confPathes[n]);
			return -1;
		}

		i = 0;
		while (fscanf(fd, "%s\n", exclude) != EOF) {
			if (sprintf(excludes[i], "%s", exclude) < 0) {
				warn("Cant read exclude name. sprintf:");
				continue;
			}
			i++;
			printf("%s\n", excludes[i-1]);
		}

		if (i == 0) {
			excludes[0][0] = '\0';
		}

		fclose(fd);

		return 0;
	}

	return -1;
}

int
getGameID()
{
	int i;

	for (i = 0; gameStartPoint[i][0]; i++) {
		if (strcmp(gameName, gameStartPoint[i]) == 0) {
			return i;
		}
	}

	return 0;
}

int
setStartPoint(char *path)
{
	int id;

	if ((id=getGameID()) == 0) {
			return 0;
	}

	if (snprintf(gameStartPoint[id], sizeof(char) * PATH_MAX,
			"%s", path) < 0) {
		warn("Cant write start point for: %s", gameName);
		return 0;
	}

	return 1;
}

int
checkStartPoint(char *path)
{
	int i;
	char *base;
	char sh[MAX_FILE_NAME+3], x86_64[MAX_FILE_NAME+7], x64[MAX_FILE_NAME+4], x86[MAX_FILE_NAME+4];

	base = basename(path);

	if (gameName[0])
	{
		if (sprintf(sh, "%s.sh", gameName) < 0)
			warn("sprintf:");
		if (sprintf(x86_64, "%s.x86_64", gameName) < 0)
			warn("sprintf:");
		if (sprintf(x64, "%s.x64", gameName) < 0)
			warn("sprintf:");
		if (sprintf(x86, "%s.x86", gameName) < 0)
			warn("sprintf:");
	}

	char *spPattern[19] = { "start.sh", "start",
		"run.sh", "run-game.sh", "rungame.sh",
		"run", "runit", "run-game", "rungame",
		gameName, sh, x86_64, x64, x86,
		"launcher", "launcher.sh", "launcher.x64", "launcher.x86_64", "launcher.x86" };

	if (isExecuteble(path)) {
		for (i = 0; i < 14; i++) {
			if (strcasecmp(base, spPattern[i]) == 0) {
				printf("GM: %s\tSP: %s\n", gameName, base);
				return 1;
			}
		}
	}

	return 0;
}

int
getPathDepth(char *path)
{
	int i, depth = 0;

	for (i = 0; path[i]; i++) {
		if (path[i] == '/') {
			depth++;
		}
	}
	
	return depth;
}

char *
searchInDir(char *root, int depth)
{
	char *file_path = NULL;
	char rpath[PATH_MAX];

	DIR *d;
	struct dirent *dir;

	if ((d = opendir(root)) == NULL) {
		warn("FATAL. Cant open dir: %s", root);
		return NULL;
	}

	setGameName(root, depth);
	/* printf("GN: %s\n", gameName); */

	while ((dir = readdir(d)) != NULL)
	{	
		//skip excludes
		if (isExcludeName(dir->d_name)) {
			continue;
		}

		if (!isDotName(dir->d_name))
		{
			getRPath(dir->d_name, root, rpath);

			if (!isDirectory(rpath)) {
				if (checkStartPoint(rpath)) {
					continue;
				}
			}
			else if (isOtherDirectory(root, rpath)) {
				searchInDir(rpath, depth);
			}
		}
	}

	if (closedir(d) == -1) {
		warn("FATAL. Cant close dir: %s", root);
		return NULL;
	}
	
	return file_path;
}

int
scan(char *path)
{
	int entries_count = 0;
	char rpath[PATH_MAX];

	if (realpath(path, rpath) == NULL) {
		warn("realpath:");
		return -1;
	}

	printf("Root: %s\n", rpath);

	if(!isDirectory(rpath)) {
		warn("\"%s\": its not directory", rpath);
		return -1;
	}

	readExcludes();
	searchInDir(rpath,  getPathDepth(rpath));

	return entries_count;
}

static void
usage()
{
	die("[-c|--config] <DIR>(<DIR>...)");
}

int
main(int argc, char **argv)
{
	char *userConf = NULL;

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
				usage();
			}
	}

	if (! *argv) {
		usage();
		die("No directory path specified");
	}

	//add check
	sprintf(confPathes[0], "%s/%s", getenv("HOME"), ".ga-org.conf");
	sprintf(confPathes[1], "%s/%s", getenv("HOME"), ".config/ga-org.conf");
	if (userConf) {
		sprintf(confPathes[2], "%s", userConf);
	}

	//add multi path option
	scan(*argv);

	return 0;
}
