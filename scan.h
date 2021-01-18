#ifndef _SCAN_H_
#define _SCAN_H_

#include "global.h"

char *searchSP(const char *location, const char *gameName);
int findGames(const char *path, struct Game_rec **Games);
int scan(const char *path, struct Game_rec **Games);

#endif
