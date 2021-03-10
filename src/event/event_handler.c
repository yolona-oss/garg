#include <time.h>
#include <ncurses.h>

#include "garg_event.h"
#include "../utils/eprintf.h"
#include "../utils/garg_time.h"

static struct timespec g_timer;
static unsigned int g_interval;

void
setup_timer(unsigned int interval)
{
	g_timer = now();
	g_interval = interval;
}

static void
update_timer()
{
	g_timer = now();
}

/* return 1 if time passed 
 * and raising TIMER event flag */
int
check_timer()
{
	struct timespec remain;
	remain = time_remain(&g_timer, g_interval);
	if (remain.tv_sec > 0) {
		return 0;
	}
	
	update_timer();
	up_event(TIMER);
	return 1;
}

int
check_input()
{
	char ch;
	if ((ch = getch()) != ERR) {
		add_event_key(INPUT, ch);
		up_event(INPUT);
		return 1;
	}

	return 0;
}
