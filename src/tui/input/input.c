#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "../../utils/util.h"
#include "../../utils/eprintf.h"

#include "../tui.h"

#define I_BUF_MAX 24

/* vars */
static char i_buf[I_BUF_MAX+1] = {0};
static char i_num_buf[12] = {0};

/* funcs */
static char start_b(char *buf);
static int  len_b(char *buf);
static int  append_b(char *buf, char c);
static void drop_b(char *buf);
static char *user_command(const char *prompt);

static int
len_b(char *buf)
{
	int i = 0;
	while (buf[i++] != '\0')
		;
	return i;
}

/* returns first char in i_buf */
static char
start_b(char *buf)
{
	return buf[0];
}

/* appending i_buf if can.
 * and return 0 if can,
 * otherwise 1 */
static int
append_b(char *buf, char c)
{
	int len = len_b(buf);
	if (len < I_BUF_MAX) {
		buf[len-1] = c;
		buf[len] = '\0';
		return 0;
	}

	return 1;
}

static void
drop_b(char *buf)
{
	buf[0] = '\0';
}

static char *
user_command(const char *prompt)
{
	int row, col;

	getmaxyx(stdscr, row, col);

	WINDOW *win = newwin(1, col, row-1, 0);

	wclrtoeol(win);
	wprintw(win, prompt);
	wrefresh(win);
	curs_set(1);

	char *cmd = (char *)ecalloc(col, 1);
	int ch;
	int exit, term;
	int pos, input_len, curs, curs_max;
	exit = term = 0;
	pos = 0;
	curs = strlen(prompt);
	curs_max = col;
	input_len = 0;
	while ((ch=mvwgetch(win, 0, curs)) != ERR)
	{
		if ((isgraph(ch) || isspace(ch))
				&& input_len < col)
		{
			if (pos == input_len)
			{
				mvwaddch(win, 0, curs, ch);
				cmd[pos] = ch;
				cmd[pos+1] = '\0';
			} else if (pos < input_len)
			{
				memmove(cmd+pos+1, cmd+pos,
					   input_len-pos);
				cmd[pos] = ch;
			}
			pos++;
			curs++;
			input_len++;
		}

		switch (ch)
		{
			/* exit if ESC pressed two times in a row */
			case KEY_ESC:
				if (term == 1) {
					break;
				}
				term = 1;
				break;
			case KEY_BACKSPACE:
				if (curs == 1) {
					term = 1;
				}
				cmd[--pos] = '\0';
				curs--;
				input_len--;
				mvwaddch(win, 0, curs, ' ');
				break;

			case KEY_ENTER:
				exit = 1;
				break;
			case CTRL('j'):
				exit = 1;
				break;

			case KEY_LEFT:
				if (curs > 1) {
					curs--;
					pos--;
				}
				break;
			case KEY_RIGHT:
				if (curs < curs_max &&
						curs <= input_len) {
					pos++;
					curs++;
				}
				break;
		}

		mvwaddstr(win, 0, 1, cmd);
		wrefresh(win);

		if (exit == 1 || term == 1) {
			break;
		}
	}

	curs_set(0);
	delwin(win);

	return (term == 1) ? NULL : cmd;
}

aval_t *
key_to_action(const char key)
{
	aval_t *aval;
	aval = (aval_t *)ecalloc(1, sizeof*aval);
	aval->action = NOTHING;
	aval->val = 0;

	char *str;

	if (isdigit(key)) {
		append_b(i_num_buf, key);
	} else {
		int val = atoi(i_num_buf);
		switch (key)
		{
			/* single */
			case 'k':
				aval->action = M_UP;
				aval->val = val;
				break;
			case 'j':
				aval->action = M_DOWN;
				aval->val = val;
				break;
			case 'l':
				aval->action = SELECT_ITEM;
				break;
			case CTRL('j'):
				aval->action = SELECT_ITEM;
				break;
			case 'e':
				aval->action = ITEM_INFO_TOGGLE;
				break;
			case 'E':
				aval->action = EDIT_ITEM;
				break;

			case 'd':
				aval->action = DELETE_ITEM;
				//TODO confirmation
				break;

			case 'q':
				//TODO confirmation
				done = 1;
				break;
			case 'Q':
				done = 1;
				break;

			/* sequences */
			case CTRL('f'):
				aval->action = M_DPAGE;
				break;
			case CTRL('b'):
				aval->action = M_UPAGE;
				break;

			case 'g':
				;
				int len = len_b(i_buf);
				if (len == 1) {
					append_b(i_buf, 'g');
				} else if (len == 2) {
					if (start_b(i_buf) == 'g') {
						aval->action = M_FIRST;
					}
					drop_b(i_buf);
				}
				break;
			case 'G':
				aval->action = M_LAST;
				break;

			case KEY_ESC:
				aval->action = ESC;
				break;

			/* input */
			case ':': //TODO
				str = user_command(":");
				add_str_status_buf(1, str);
				free(str);
				break;
			case '/':
				aval->action = FIND_ITEM;
				break;
		}

		drop_b(i_num_buf);
	}

	return aval;
}
