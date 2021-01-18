#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <unistd.h>
#include <libgen.h>
#include <regex.h>
#include <limits.h>
#include <sys/stat.h>

#include "global.h"
#include "util.h"

char *argv0;

int g_qflag, g_dflag;

void
verr(const char *fmt, va_list ap)
{
	if (!g_qflag) {
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

		fflush(stderr);
	}
}

void
allerMsg(const char *fmt, ...)
{
	if (g_dflag) {
		va_list ap;
		
		va_start(ap, fmt);
		vfprintf(stdout, fmt, ap);
		va_end(ap);

		fflush(stdout);
	}
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
	for (int i = 0; i < n; i++) {
		printf("%s%s", *pa++, sep);
	}

	putchar('\n');
}

void
freePP(char **pa, int n)
{
	if (pa) {
		while (n--) {
			free(pa[n]);
		}
		free(pa);
	}
}

void
freePSG(struct Game_rec *Game)
{
	if (Game) {
		Game->id = -1707;
		if (Game->name)
			free(Game->name);
		if (Game->location)
			free(Game->location);
		if (Game->start_point)
			free(Game->start_point);
		free(Game);
	}
}

void
freePPSG(struct Game_rec **Games)
{
	if (Games) {
		for (int i = 0; Games[i] && (Games[i]->location ||
						Games[i]->name ||
						Games[i]->start_point); i++) {
			freePSG(Games[i]);
		}
		free(Games);
	}
}

int
rmDupInPP(char **pa, int n)
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
	FILE *fd;

	if ((fd=fopen(path, "r"))) {
		fclose(fd);
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
	int status;
	char *name;
	regex_t regex;

	name = basename((char*)path);

	for (int i = 0; exceptionName[i]; i++) {
		allerMsg(" [!!] Start read exceptions\n");
		status = regcomp(&regex, exceptionName[i], REG_EXTENDED|REG_NEWLINE|REG_NOSUB);

		if (status) {
			warn("regcomp:");
			regfree(&regex);
			continue;
		}

		allerMsg("   [!!] Comparing\n");
		status = regexec(&regex, name, 0, NULL, 0);
		
		if (!status) {
			allerMsg(" [!!] Exception check END\n");
			regfree(&regex);
			return 1;
		}

		regfree(&regex);
	}

	allerMsg(" [!!] Exception check END\n");
	return 0;
}

int
isStartPoint(const char *filePath, const char *gameName)
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
		(char *)gameName, sh, x86_64, x64, x86,
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
isGameEntryUniq(struct Game_rec **ppGames, struct Game_rec *pGame)
{
	int cached_count = countGameEntries(ppGames);
	if (ppGames && pGame) {
		for (int i = 0; ppGames[i]; i++) {
			if (gecmp(ppGames[i], pGame) == 0) {
				return 0;
			}
		}
	}

	return 1;
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

	if (pp) {
		while (*pp++)
			ret++;
	}

	return ret;
}

void
printGameEntry(struct Game_rec *Game)
{
	if (Game) {
		printf("\n######################################\n");
		printf(" id          - %d\n", Game->id);
		printf(" name        - %s\n", Game->name);
		printf(" location    - %s\n", Game->location);
		printf(" start point - %s\n", Game->start_point);
		printf("######################################\n\n");

		fflush(stdout);
	} else {
		warn("Cant print game entry: entry dose not exist");
	}
}

int
isBrokenGameEntry(struct Game_rec *Game)
{
	short eflag = 0;

	if (!Game) {
		return -1;
	}

	if (!isExist(Game->location)) {
		/* warn("Game location dont exist"); */
		eflag |= G_NOLOC;
	}
		
	if (!isStartPoint(Game->start_point, Game->name)) {
		/* warn("Game start point dont exist"); */
		eflag |= G_NOSP;
	}

	if (!Game->name) {
		/* warn("Game dont hame name"); */
		eflag |= G_NONAME;
	}

	return eflag;
}

int
add_game(Game_rec newrec)
{
	Game_rec *grp;

	if(gr_tab.game_rec == NULL) {
		gr_tab.game_rec =
			(Game_rec *)malloc(GR_INIT * sizeof(Game_rec));
		if (gr_tab.game_rec == NULL)
			return -1;
		gr_tab.max = GR_INIT;
		gr_tab.ngames = 0;
	} else if (gr_tab.ngames >= gr_tab.max) {
		grp = (Game_rec *)realloc(gr_tab.game_rec,
				(GR_GROW*gr_tab.max) * sizeof(Game_rec));
		if (grp == NULL)
			return -1;
		gr_tab.max *= GR_GROW;
		gr_tab.game_rec = grp;
	}
	gr_tab.game_rec[gr_tab.ngames] = newrec;

	return gr_tab.ngames++;
}

int
del_game(int id)
{
	int i;

	for (i = 0; i < gr_tab.max; i++) {
		if (gr_tab.game_rec[i].id == id) {
			memmove(gr_tab.game_rec+i, gr_tab.game_rec+i+1,
					(gr_tab.ngames-(i+1)) * sizeof(Game_rec));
			gr_tab.ngames--;
			return 1;
		}
	}

	return 0;
}


int
addGameEntry(struct Game_rec *Game, const char *name, const char *location, const char *startPoint)
{
	if (!Game) {
		warn("NO memory allocated for new game entry");
		return -1;
	}

	Game->id = 0;

	return editGameEntry(Game, name, location, startPoint);
}

int
editGameEntry(struct Game_rec *Game, const char *name, const char *location, const char *startPoint)
{
	int len, n = 0;

	if (name) {
		len = strlen(name) + 1;

		Game->name = (char *)realloc(Game->name, len);
		if (Game->name) {
			memcpy(Game->name, name, len);
			n++;
		}
	}

	if (location) {
		len = strlen(location) + 1;

		Game->location = (char *)realloc(Game->location, len);
		if (Game->location) {
			memcpy(Game->location, location, len);
			n++;
		}
	}

	if (startPoint) {
		len = strlen(startPoint) + 1;

		Game->start_point = (char *)realloc(Game->start_point, len);
		if (Game->start_point) {
			memcpy(Game->start_point, startPoint, len);
			n++;
		}
	}

	return n;
}

int
countGameEntries(struct Game_rec **Games)
{
	int c;
	for (c = 0; Games[c]; c++)
		;

	return c;
}

int
gecmp(struct Game_rec *src, struct Game_rec *dst)
{
	if (src->location || dst->location ||
		src->name || dst->name ||
		src->start_point || dst->start_point) {
		return -1;
	}

	if (strcmp(src->location, dst->location) == 0 &&
			strcmp(src->name, dst->name) == 0 &&
			strcmp(src->start_point, dst->start_point) == 0) {
		return 0;
	}

	return 1;
}


int
gecpy(struct Game_rec *dst, struct Game_rec *src)
{
	return editGameEntry(dst, src->name, src->location, src->start_point);
}
