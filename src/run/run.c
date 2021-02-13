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

void
resizeHandler(int sig)
{
	getmaxyx(stdscr, g_scr_h, g_scr_w);
}

static int
pre(void)
{
	signal(SIGWINCH, resizeHandler);

	initscr();
	show_init_scr();

	db_read_settings();
	db_read_cached_recs();
	/* scan_inclusions(); */

	if (g_user_path[0]) {
		scan(g_user_path);
	}

	return 0;
}

int
run()
{
	if ( pre() ) {
		warn("Cant pre initiate run");
		return -1;
	}
	
	struct timespec start, current, diff, intspec, wait;
	int event;
	int key;

	nodelay(stdscr, true);
	while (!done) {
		if (clock_gettime(CLOCK_MONOTONIC, &start) < 0) {
			die("clock_gettime:");
		}
		
		noecho();
		if ((key = getch()) != ERR) {
			switch (key)
			{
				case 'k':
					waddstr(stdscr, "UP\n");
					break;
				case 'j':
					waddstr(stdscr, "DOWN\n");
					break;
				case 'h':
					waddstr(stdscr, "LEFT\n");
					break;
				case 'l':
					waddstr(stdscr, "RIGHT\n");
					break;
			}
		}
		echo();

		/* EVENT HENDLING */
		while ((event=pull_event()) >= 0) {
			switch (event)
			{
			}
		}

		/* UPDATER */
		update();

		wrefresh(stdscr);
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
