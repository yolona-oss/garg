#include <signal.h>
#include <stdlib.h>

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
static menu_t *pre(void);
static int post(menu_t *menu);

static menu_t * 
pre(void)
{
	/* setupping resize handler */
	/* signal(SIGWINCH, resizeHandler); */

	/* events */
	event_action(INPUT, check_input);
	event_action(TIMER, check_timer);

	init_tui();

	db_read_settings();
	db_read_cached_recs();
	/* scan_inclusions(); */

	if (g_user_path[0]) {
		scan(g_user_path);
	}

	return init_game_menu();
}

static int
post(menu_t *menu)
{
	destroy_tui();
	destroy_game_menu(menu);

	return 0;
}

#include "../utils/util.h"
int
run()
{
	menu_t *menu = pre();

	struct timespec start, wait;
	event_t event;
	aval_t *av;

	setup_timer(M_SEC(4));

	while (!done) {
		start = now();

		show_header();
		show_status_bar();
		menu_move(menu, NULL);

		/* EVENT HENDLING */
		while (poll_event(&event) != -1) {
			switch (event.type)
			{
				case TIMER:
					add_str_status_buf(2, "TIMER");
					check_gr_tab();
					break;
				case INPUT:
					av = key_to_action(event.key);
					add_str_status_buf(2, "INPUT");
					menu_move(menu, av);
					free(av);
					break;

				default:
					add_str_status_buf(2, "def");
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

	post(menu);

	return 0;
}
