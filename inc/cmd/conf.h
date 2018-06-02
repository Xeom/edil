#if !defined(CMD_CONF_H)
# define CMD_CONF_H
# include "file.h"
# include "win.h"
# include "vec.h"

void conf_run_file(file *f, win *w);

void conf_cmd_run_file(vec *rtn, vec *args, win *w);

#endif