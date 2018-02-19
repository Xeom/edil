#if !defined(CMD_H)
# define CMD_H
# include "cli.h"

typedef enum
{
    cmd_mode_bar,
    cmd_mode_buf,
    cmd_mode_opt,
    cmd_mode_mnu,
    cmd_mode_sel,
    cmd_mode_rct,
    cmd_mode_nummodes
} cmd_mode;

extern char *cmd_mode_names[cmd_mode_nummodes];
extern cmd_mode cmd_cur_mode;
extern vec cmd_bar_str;

void cmd_init(void);
void cmd_kill(void);

void text_buf_update_cur(text_buf *b, text_cur *cur);
void cmd_arrow(cli_key key);
void cmd_ins(cli_key key);

void cmd_ins_flush(void);

void cmd_handle_key(cli_key key);
#endif
