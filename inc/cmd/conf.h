#if !defined(CMD_CONF_H)
# define CMD_CONF_H
# include "types.h"

void conf_run_file(file *f, win *w);

void conf_run_file_name(char *name, win *w);

void conf_run_default_files(win *w);

void cmd_conf_init(void);

#endif
