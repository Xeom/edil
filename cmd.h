#if !defined(CMD_H)
# define CMD_H
# include "cli.h"

void cmd_init(void);

void text_buf_update_cur(text_buf *b, text_cur *cur);
void cmd_arrow(cli_key key);
void cmd_ins(cli_key key);

void cmd_ins_flush(void);

void cmd_handle_key(cli_key key);
#endif
