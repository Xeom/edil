#if !defined(CMD_CONF_H)
# define CMD_CONF_H
# include "file.h"
# include "win.h"
# include "vec.h"

void conf_run_file(file *f, win *w);

void conf_run_file_name(char *name, win *w);

void conf_run_default_files(win *w);

void conf_cmd_run_file(vec *rtn, vec *args, win *w);
void conf_cmd_remap(vec *rtn, vec *args, win *w);
void conf_cmd_unmap(vec *rtn, vec *args, win *w);
void conf_cmd_translate(vec *rtn, vec *args, win *w);

#endif
