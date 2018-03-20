#if !defined(CMD_FILE_H)
# define CMD_FILE_H
# include "win.h"
# include "vec.h"

void file_cmd_load  (vec *rtn, vec *args, win *w);
void file_cmd_revert(vec *rtn, vec *args, win *w);
void file_cmd_dump  (vec *rtn, vec *args, win *w);
void file_cmd_chdir(vec *rtn, vec *args, win *w);

#endif
