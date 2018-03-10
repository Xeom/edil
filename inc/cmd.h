#if !defined(CMD_H)
# define CMD_H
# include <stdarg.h>
# include "vec.h"

typedef struct cmd_info_s cmd_info;

typedef enum
{
    cmd_pipe_chrs,
    cmd_pipe_file,
    cmd_pipe_loc,
    cmd_pipe_none,
    cmd_pipe_cmd
} cmd_pipe_type;

struct cmd_info_s
{
    char *name;
    void (*funct)(vec *rtn, vec *args);
};

extern vec cmd_infos;

void cmd_format(vec *chrs, char *fmt, ...);

void cmd_init(void);

void cmd_kill(void);

void cmd_run(vec *args, vec *rtn);

void cmd_parse(vec *args, vec *chrs, size_t ind);

#endif
