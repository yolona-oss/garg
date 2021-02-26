#include <signal.h>

#include "../main.h"
#include "../games/gamerec.h"
#include "../scan/scan.h"
#include "../db/dbman.h"
#include "../utils/eprintf.h"
#include "../utils/garg_time.h"
#include "../event/garg_event.h"

#include "../tui/tui.h"

#define _FPS 60

#define M_SEC(n) 1000*n

/* vars */
static const unsigned int g_main_interval = M_SEC(1/_FPS);

/* funcs */
static int pre(void);
static int post(void);

static int
pre(void)
{
	/* setupping resize handler */
	signal(SIGWINCH, resizeHandler);

	/* events */
	event_action(TIMER, check_timer);
	event_action(INPUT, check_input);

	init_tui();

	db_read_settings();
	db_read_cached_recs();
	/* scan_inclusions(); */

	if (g_user_path[0]) {
		scan(g_user_path);
	}

	init_game_menu();

	return 0;
}

static int
post(void)
{
	destroy_tui();

	return 0;
}

int
run()
{
	if ( pre() ) {
		warn("Cant pre initiate run");
		return 1;
	}
	
	struct timespec start, wait;
	enum EVENT_ID event;

	setup_timer(M_SEC(4));

	while (!done) {
		start = now();

		show_header();
		show_status_bar();

		/* EVENT HENDLING */
		while ((event=pull_event()) != -1) {
			switch (event)
			{
				case TIMER:
					check_gr_tab();
					break;
				case INPUT:
					menu_move(input_command());
					break;

				default:
					break;
			}
		}

		/* UPDATER */
		doupdate();

		if (!done) {
			wait = time_remain(&start, g_main_interval);
			wait_left(wait);
		}
	}

	post();

	return 0;
}
