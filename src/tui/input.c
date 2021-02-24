#include "tui.h"

#define I_BUF_MAX 24

static char i_buf[I_BUF_MAX+1] = {0};

/* returns index of last char in i_buf */
int
len_b(void)
{
	int i = 0;
	while (i_buf[i++] != '\0')
		;
	return i;
}

/* returns first char in i_buf */
char
start_b(void)
{
	return i_buf[0];
}

/* appending i_buf if can.
 * and return 0 if can,
 * otherwise 1 */
int
append_b(char c)
{
	int len = len_b();
	/* if (len < I_BUF_MAX) { */
		i_buf[len-1] = c;
		i_buf[len] = '\0';
		return 0;
	/* } */

	/* return 1; */
}

void
drop_b(void)
{
	i_buf[0] = '\0';
}

enum MENU_ACT
input_command(void)
{
	enum MENU_ACT action = NOTHING;
	int key;

	if ((key = getch()) != ERR) {
		switch (key)
		{
			/* single */
			case 'k':
				action = M_UP;
				break;
			case 'j':
				action = M_DOWN;
				break;
			case 'l':
				action = SELECT;
				break;
			case CTRL('f'):
				action = M_DPAGE;
				break;
			case CTRL('b'):
				action = M_UPAGE;
				break;
			case 'G':
				action = M_LAST;
				break;

			case 'q':
				done = 1;
				break;

			/* sequences */
			case 'g':
				;
				int len = len_b();
				if (len == 1) {
					append_b('g');
				} else if (len == 2) {
					if (start_b() == 'g') {
						action = M_FIRST;
					}
					drop_b();
				}
				break;
		}
	}

	return action;
}
