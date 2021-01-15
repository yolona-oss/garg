#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <libgen.h>
#include <regex.h>
#include <limits.h>
#include <sys/stat.h>

#include "global.h"
#include "util.h"

char *argv0;
char *gameName;

void
verr(const char *fmt, va_list ap)
{
	if (argv0 && strncmp(fmt, "usage", sizeof("usage") - 1)) {
		fprintf(stderr, "%s: ", argv0);
	}

	vfprintf(stderr, fmt, ap);

	if (fmt[0] && fmt[strlen(fmt) - 1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else {
		fputc('\n', stderr);
	}

	fflush(stdout);
}

void
warn(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	verr(fmt, ap);
	va_end(ap);
}

void
die(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	verr(fmt, ap);
	va_end(ap);

	exit(1);
}

void
printPP(char **pa, char *sep, int n)
{
	printf(" [II] Printting array of pointers\n");
	for (int i = 0; i < n; i++) {
		printf("%s%s", *pa++, sep);
	}

	printf("\n");
	fflush(stdout);
}

void
freePP(char **pa, int n)
{
	printf("\n");
	while (n--) {
		printf(" [%3d] free - %s\n", n, pa[n]); fflush(stdout);
		free(pa[n]);
	}
	printf(" [END] free pp\n"); fflush(stdout);
	printf("\n");
	free(pa);
}

void
freeSG(struct game Game[])
{
	printf("Freeing SG\n");
	for (int i = 0; Game[i].location; i++) {
		Game[i].id = -1707;
		free(Game[i].name);
		free(Game[i].location);
		free(Game[i].starPoint);
	}
	printf("SG freeing success\n");
}

int
rmDupInArrOfPointers(char *pa[], int n)
{ 
    if (n == 0 || n == 1)
        return n;
 
    char *uniq[n];
	char *current;

	int k=0;
	int i, j;
	for(i=0; i<n; i++) {
	   if( pa[i] == NULL ) {
			continue;
		}
		current = pa[i];
		uniq[k] = current;
		k++;
		for(j=i+1; j<n; j++) {
			if( current == pa[j] ) {
				pa[j] = NULL;
			}
		}
	}

	for (j = i = 0; i < n; i++) {
		if (i > k-1) {
			pa[i] = NULL;
		} else {
			pa[i] = uniq[j++];
		}
	}
 
    return k-1;
}

int
isExist(const char *path)
{
	if (!access(path, F_OK)) {
		return 1;
	}

	return 0;
}

int
isDirectory(const char *path)
{
	struct stat fileStat;

	if (stat(path, &fileStat) == -1) {
		warn("Cant get stats of file: %s:", path);
		return 0;
	}

	if (S_ISDIR(fileStat.st_mode)) {
		return 1;
	}

	return 0;
}

int
isOtherDirectory(const char *root, const char *path)
{
	if (!isDirectory(path)) {
		return 0;
	}

	if (isDotName(path) || strcmp(root, path) == 0) {
		return 0;
	}

	return 1;
}

int
isDotName(const char *name)
{
	if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
		return 1;
	}

	return 0;
}

int
isExecuteble(const char *path)
{
	if (access(path, X_OK) == 0) {
		return 1;
	}

	return 0;
}

int
isExcludeName(const char *path)
{
	return 0;
	/* int status; */
	/* char *name; */
	/* regex_t regex; */

	/* char tmp[1000]; */

	/* name = basename((char*)path); */

	/* for (int i = 0; exceptionName[i]; i++) { */
	/* 	printf(" [!!] Start read exceptions\n"); fflush(stdout); */

	/* 	/1* strcpy(tmp, exceptionName[i]); *1/ */
	/* 	/1* printf(" %s\n", tmp); fflush(stdout); *1/ */
	/* 	/1* status = regcomp(&regex, tmp, REG_EXTENDED|REG_NEWLINE|REG_NOSUB); *1/ */

	/* 	status = regcomp(&regex, exceptionName[i], REG_EXTENDED|REG_NEWLINE|REG_NOSUB); */

	/* 	if (status) { */
	/* 		warn("regcomp:"); */
	/* 		regfree(&regex); */
	/* 		continue; */
	/* 	} */

	/* 	printf("   [!!] Comparing\n"); fflush(stdout); */
	/* 	status = regexec(&regex, name, 0, NULL, 0); */
		
	/* 	/1* printf(" [!!] \n"); fflush(stdout); *1/ */
	/* 	if (!status) { */
	/* 		printf(" [!!] Exception check END\n"); fflush(stdout); */
	/* 		regfree(&regex); */
	/* 		return 1; */
	/* 	} */

	/* 	regfree(&regex); */
	/* } */

	/* printf(" [!!] Exception check END\n"); fflush(stdout); */
	/* return 0; */
}

int
isStartPoint(const char *filePath)
{
	int i;
	char *fileName;
	char sh[FILENAME_MAX], x86_64[FILENAME_MAX], x64[FILENAME_MAX], x86[FILENAME_MAX];

	fileName = basename((char*)filePath);

	if (sprintf(sh, "%s.sh", gameName) < 0)         { warn("sprintf:"); }
	if (sprintf(x86_64, "%s.x86_64", gameName) < 0) { warn("sprintf:"); }
	if (sprintf(x64, "%s.x64", gameName) < 0)       { warn("sprintf:"); }
	if (sprintf(x86, "%s.x86", gameName) < 0)       { warn("sprintf:"); }

	char *spPattern[21] = { "start.sh", "start",
		"run.sh", "run-game.sh", "rungame.sh",
		"run", "runit", "run-game", "rungame",
		"runme", "runme.sh",
		gameName, sh, x86_64, x64, x86,
		"launcher", "launcher.sh", "launcher.x64", "launcher.x86_64", "launcher.x86" };

	if (isExecuteble(filePath)) {
		for (i = 0; i < 21; i++) {
			if (strcasecmp(fileName, spPattern[i]) == 0) {
				return 1;
			}
		}
	}

	return 0;
}

int
getRPath(const char *fileName, const char *root, char *rpath)
{
	char path[PATH_MAX];

	if ((sprintf(path,
			"%s/%s", root, fileName)) < 1) {
		warn("Cant fill path:");
		return 0;
	}

	if (!realpath(path, rpath)) {
		warn("realpath: \"%s\":", path);
		return 0;
	}

	return 1;
}

int
getLenOfPP(char **pp)
{
	int ret = 0;

	while (*pp++)
		ret++;

	return ret;
}

void
printGameEntry(int id)
{
	printf("\n######################################\n");
	printf(" id          - %d\n", id);
	printf(" name        - %s\n", Game[id].name);
	printf(" location    - %s\n", Game[id].location);
	printf(" start point - %s\n", Game[id].starPoint);
	printf("######################################\n\n");
}

int
editGameEntry(int id, const char *name, const char *location, const char *startPoint)
{
	if (id < 0 || id > MAX_GAMES) {
		warn("ID of game entry is out of range");
		return -1;
	}

	int len;

	if (name) {
		len = strlen(name) + 1;

		if (Game[id].name) {
			free(Game[id].name);
		}

		Game[id].name = (char *)malloc(len);
		if (Game[id].name)
			memcpy(Game[id].name, name, len);
	}

	if (location) {
		len = strlen(location) + 1;

		if (Game[id].location) {
			free(Game[id].location);
		}

		Game[id].location = (char *)malloc(len);
		if (Game[id].location)
			memcpy(Game[id].location, location, len);
	}

	if (startPoint) {
		len = strlen(startPoint) + 1;

		if (Game[id].starPoint) {
			free(Game[id].starPoint);
		}

		Game[id].starPoint = (char *)malloc(len);
		if (Game[id].starPoint)
			memcpy(Game[id].starPoint, startPoint, len);
	}

	return 1;
}

