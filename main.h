#ifndef _MAIN_H_
#define _MAIN_H_

/* vars */
extern char *argv0;
extern int g_qflag, g_dflag;

extern char *g_user_db;

extern char **exceptionName;
extern char **exceptionPath;
extern char **inclusions;

extern struct Gr_tab gr_tab;

/* functions */
int scan(const char *path);
void check_gr_tab(struct Gr_tab gr_tab);

#endif
