#if !defined(CMD_H)
# define CMD_H
# include <stdarg.h>
# include "vec.h"
# include "win.h"

typedef enum
{
    cmd_pipe_chrs,
    cmd_pipe_file,
    cmd_pipe_loc,
    cmd_pipe_none,
    cmd_pipe_cmd
} cmd_pipe_type;

extern vec cmd_items;

void cmd_init(void);
void cmd_kill(void);

void cmd_run(vec *args, vec *rtn, win *w);

void cmd_parse(vec *args, vec *chrs, size_t ind);

void cmd_log(vec *chrs, int iscmd);
#endif
