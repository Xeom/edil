#if !defined(BUF_CMD_INFO)
# define BUF_CMD_INFO
# include "vec.h"
# include "win.h"

void buf_cmd_info(vec *rtn, vec *args, win *w);

void buf_cmd_next(vec *rtn, vec *args, win *w);

void buf_cmd_prev(vec *rtn, vec *args, win *w);

void buf_cmd_kill(vec *rtn, vec *args, win *w);

void buf_cmd_quit(vec *rtn, vec *args, win *w);

#endif
