#if !defined(CMD_H)
# define CMD_H
# include "cli.h"

void cmd_init(void);

void cmd_arrow(cli_key key);
void cmd_ins(cli_key key);

void cmd_ins_flush(void);

void cmd_handle_key(cli_key key);
#endif
