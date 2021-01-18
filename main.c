#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "global.h"
#include "util.h"
#include "scan.h"

char *userConf;

static void
usage(void)
{
	die("[-q|--quiet] [-c|--config] <DIR>(<DIR>...)");
}

int
main(int argc, char **argv)
{
	argv0 = *argv;
	argv++;

	g_qflag = 0;
	g_dflag = 0;

	for (int i = 0; *argv && (*argv)[0] == '-' && (*argv)[1]; i++, argc--, argv++)
	{
		//todo
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
			g_qflag = 1;
		}
		if (strcmp(*argv, "d") == 0 ||
				strcmp(*argv, "debug") == 0) {
			g_dflag = 1;
		} else if (strcmp(*argv, "h") == 0 ||
				strcmp(*argv, "help") == 0) {
			usage();
		} else {
			warn("Unknown argument: \"%s\"", *argv);
			usage();
		}
	}

	//try to use prepare()
	int i;
	struct Game_rec **Games;

	if (!(Games = (struct Game_rec **)calloc(MAX_GAMES+1, sizeof(struct Game_rec *)))) {
		warn("Cant alloc memory for Game struct\nmalloc:");
		return 1;
	}

	//add multi path option
	scan((*argv) ? *argv : ".", Games);

	printf("##########[!END!]############");
	i=0;
	while (Games[i]) {
		printGameEntry(Games[i++]);
	}

	freePPSG(&Games[0]);
	freePP(exceptionName, getLenOfPP(exceptionName));

	return 0;
}
