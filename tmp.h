#ifndef _SCAN_H_
#define _SCAN_H_

#include "gamerec.h"
#include "scan_spec.h"

#define C_SPEC 1

typedef struct spec_name spec_name_t;
struct spec_name {
	const char *val;
	char *(*func)(const char *);
};

/* static const spec_name_t special_names[C_SPEC]; */

/* funcs */
char *get_special_scanner(int i);

/* special scans */
char *scan_steam(const char *path);

/* vars */
extern game_tab_t gr_tab;
extern int g_scan_depth;
extern const spec_name_t special_names[C_SPEC];

/* funcs */
int scan_inclusions(const char *path);

int scan(const char *path);
int rescan(const char *path);
int update(const char *path);

#endif
