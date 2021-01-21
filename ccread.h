#ifndef _CCREAD_H_
#define _CCREAD_H_

#define DEF_CACH_PTH ".cache/ga-org.conf"

#define MAX_EXCEPTIONS  1000
#define MAX_INCLUSIONS  MAX_EXCEPTIONS

#define _config_err(cfg) warn("Reading cache file: \"%s\" error:%d - %s", config_error_file(cfg), config_error_line(cfg), config_error_text(cfg));
#define _config_write_options CONFIG_OPTION_COLON_ASSIGNMENT_FOR_GROUPS\
							  |CONFIG_OPTION_OPEN_BRACE_ON_SEPARATE_LINE\
							  |CONFIG_OPTION_FSYNC

/* funcs */
int cache_delete_record(const char *path, int n);

struct Game_rec *cache_get_record(const char *path, int n);
int cache_put_record(const char *path, struct Game_rec *GE);
int cache_get_length(const char *path);

int readConfig(void);
int writeCache(void);
int readCache(void);

/* vars */
extern struct Gr_tab gr_tab;
extern char *userConf;

#endif
