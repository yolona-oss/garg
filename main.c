#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <getopt.h>

#include "main.h"
#include "util.h"
#include "eprintf.h"
#include "gamerec.h"

/* vars */
char *g_user_db;

char **exceptionName;
char **exceptionPath;
char **inclusions;

static void
cleanup()
{
	if (g_user_db) free(g_user_db);
	pp_free(exceptionName);
	grt_free(gr_tab);
}

static void
terminate()
{
	cleanup();
	die("Aborting.");
}

static void
usage(void)
{
	/* TODO */
	die("-c <PATH> -[qd] -s <PATH>");
}

int
main(int argc, char **argv)
{
	/* signals hendling */
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = terminate;
	sigaction(SIGTERM, &action, NULL);
	sigaction(SIGINT, &action, NULL);

	argv0 = *argv;

	g_qflag = 0;
	g_dflag = 0;

	char ch = 0;
	char path[PATH_MAX] = "";

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
				g_user_db = estrdup(optarg);
				break;
			case 'q':
				g_qflag = 1;
				break;
			case 'd':
				g_dflag = 1;
				break;
			case 's':
				strcpy(path, optarg);
				break;
			case 'h':
				usage();
				break;
			case '?':
				usage();
			default:
				usage();
				/* TODO */
				break;
		}
	}

	/* ncurece interface */

	/* add multi path option */
	scan(path[0] ? path : "./");

	printf("##########[!END!]############");
	for (int i = 0; i < gr_tab.ngames; i++) {
		gr_print(&gr_tab.game_rec[i]);
	}
	printf("\n%d\n", gr_tab.ngames);

	cleanup();

	return EXIT_SUCCESS;
}
