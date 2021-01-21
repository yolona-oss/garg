#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "main.h"
#include "util.h"
#include "eprintf.h"
#include "gamerec.h"

/* vars */
char *userConf;
char *cachePath;

char **exceptionName;
char **exceptionPath;
char **inclusions;

/* TODO */
static void
usage(void)
{
	die("-c <PATH> -[qd] <PATH>");
}

int
main(int argc, char **argv)
{
	argv0 = *argv;

	g_qflag = 0;
	g_dflag = 0;

	char ch;
	char *path = NULL;

	struct option long_options[] = {
		{ "config", required_argument, NULL, 'c' },
		{ "quiet",  no_argument,       NULL, 'q' },
		{ "debug",  no_argument,       NULL, 'd' },
		{ "help",   no_argument,       NULL, 'h' },
		{ "scan",   required_argument, NULL, 's' }
	};

	while ((ch = getopt_long(argc, argv, "s:c:qdh", long_options, NULL)) != -1) {
		switch (ch) {
			case 'c':
				userConf = estrdup(optarg);
				break;
			case 'q':
				g_qflag = 1;
				break;
			case 'd':
				g_dflag = 1;
				break;
			case 's':
				path = estrdup(optarg);
				break;
			case 'h':
				usage();
				break;
			case '?':
				usage();
			default:
				usage();
				//TODO
				break;
		}
	}

	//ncurece interface

	//add multi path option
	scan(path ? path : "./");

	printf("##########[!END!]############");
	for (int i = 0; i < gr_tab.ngames; i++) {
		gr_print(&gr_tab.game_rec[i]);
	}

	if (userConf) free(userConf);
	if (path) free(path);
	grt_free(gr_tab);
	pp_free(exceptionName, pp_get_len(exceptionName));

	return EXIT_SUCCESS;
}
