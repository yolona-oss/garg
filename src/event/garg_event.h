#ifndef _TUI_EVENT_H_
#define _TUI_EVENT_H_

#define N_EVENTS 2

enum EVENT_ID {
	TIMER = 0,
	INPUT,
};

typedef struct event_t event_t;
struct event_t {
	int flag;
	char buf[1025];
	int (*func)(void);
};

/* vars */

/* funcs */
void event_action(enum EVENT_ID eid, int (*func)(void));
void up_event(enum EVENT_ID eid);
void add_event_buf(enum EVENT_ID eid, const char *s);

int pull_event(void);

/* handlers */
void setup_timer(unsigned int interval_ms);
void drop_timer(void);
int check_timer(void);

int check_input(void);

#endif
