#ifndef _TUI_EVENT_H_
#define _TUI_EVENT_H_

#define N_EVENTS 2

enum EVENT {
	TIMER = 0,
	INPUT,
};

typedef struct event_t event_t;
struct event_t {
	int flag;
	int (*func)(void);
};

/* vars */

/* funcs */
void setup_event(enum EVENT e, int (*func)(void));
void do_event_func(enum EVENT e);

void up_event(int event);
void close_event(int event);

int pull_event();

/* handlers */
void setup_timer(unsigned int interval_ms);
void drop_timer(void);
int check_timer(void);

int check_input();

#endif
