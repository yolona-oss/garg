#include <string.h>
#include <ncurses.h>

#include "tui.h"
#include "../utils/eprintf.h"

void
cut(char *str, char *res, int width)
{
	if (strlen(str) > 1) {
		esnprintf(res, sizeof(char) * (width - 2),
				"%s", str);
		strcat(res, "...");
	} else {
		esnprintf(res, sizeof(res),
				"%s", str);
	}
}
