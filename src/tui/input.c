#include "tui.h"

static char i_buf[24] = {""};

static int
endb(void)
{
	int i = 0;
	while (i_buf[i] != '\0')
		i++;
	return i;
}

char
start_ibuf(void)
{
	return i_buf[0];
}

void
append_ibuf(char c)
{
	i_buf[endb()+1] = c;
}

void
drop_ibuf(void)
{
	i_buf[0] = '\0';
}

void
input_handle(void)
{
	enum MENU_ACT action = NOTHING;
	int key;

	if ((key = getch()) != ERR) {
		switch (key)
		{
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

			case 'q':
				done = 1;
				break;
		}

		menu_move(action);
		action = NOTHING;
	}
}
