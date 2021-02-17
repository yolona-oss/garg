#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <ncurses.h>

#include "../main.h"
#include "../games/gamerec.h"
#include "../scan/scan.h"
#include "../db/dbman.h"
#include "../utils/eprintf.h"
#include "../event/event.h"

#include "../tui/tui.h"

#define M_SEC(n) 1000*n

static const unsigned int interval = M_SEC(0.05);

extern char g_user_path[PATH_MAX];
extern int done;

static void
difftimespec(struct timespec *res, struct timespec *a, struct timespec *b)
{
	res->tv_sec = a->tv_sec - b->tv_sec - (a->tv_nsec < b->tv_nsec);
	res->tv_nsec = a->tv_nsec - b->tv_nsec +
	               (a->tv_nsec < b->tv_nsec) * 1E9;
}

static int
pre(void)
{
	/* setupping resize handler */
	signal(SIGWINCH, resizeHandler);

	init_tui();

	db_read_settings();
	db_read_cached_recs();
	/* scan_inclusions(); */

	if (g_user_path[0]) {
		scan(g_user_path);
	}

	check_gr_tab(gr_tab);
	show_game_list(IDLE);

	return 0;
}

int
run()
{
	if ( pre() ) {
		warn("Cant pre initiate run");
		return 1;
	}
	
	struct timespec start, current, diff, intspec, wait;
	int event, key;

	enum MOVEMENT direction;

	while (!done) {
		if (clock_gettime(CLOCK_MONOTONIC, &start) < 0) {
			die("clock_gettime:");
		}

		if ((key = getch()) != ERR) {
			switch (key)
			{
				case 'k':
					direction = UP;
					break;
				case 'j':
					direction = DOWN;
					break;

				case 'q':
					done = 1;
					break;
			}

			if (direction != IDLE) {
				show_game_list(direction);
			}
			direction = IDLE;
		}

		show_status_bar();

		/* EVENT HENDLING */
		while ((event=pull_event()) >= 0) {
			switch (event)
			{
			}
		}

		/* UPDATER */
		/* check_gr_tab(gr_tab); */

		if (!done) {
			if (clock_gettime(CLOCK_MONOTONIC, &current) < 0) {
				die("clock_gettime:");
			}
			difftimespec(&diff, &current, &start);

			intspec.tv_sec = interval / 1000;
			intspec.tv_nsec = (interval % 1000) * 1E6;
			difftimespec(&wait, &intspec, &diff);

			if (wait.tv_sec >= 0) {
				if (nanosleep(&wait, NULL) < 0 &&
				    errno != EINTR) {
					die("nanosleep:");
				}
			}
		}
	}

	endwin();

	return 0;
}
