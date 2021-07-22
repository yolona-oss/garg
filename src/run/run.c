#include <signal.h>
#include <stdlib.h>

#include "../main.h"
#include "../games/gamerec.h"
#include "../scan/scan.h"
#include "../db/dbman.h"
#include "../utils/eprintf.h"

#include "../gui/gui.h"

#define _FPS 60

#define M_SEC(n) 1000*n

/* vars */
static const unsigned int g_main_interval = M_SEC(1/_FPS);

/* funcs */
static void pre(void);
static void post(void);

static void
pre(void)
{
	db_read_settings();
	db_read_cached_recs();
	/* scan_inclusions(); */

	if (g_user_path[0]) {
		scan(g_user_path);
	}
	init_gui();
}

static void
post()
{
}

int
run()
{
	pre();

	while (!done) {

	}

	post();

	return 0;
}
