#if !defined(CMD_INDENT_H)
# define CMD_INDENT_H

# include "vec.h"
# include "win.h"

void indent_cmd_lvlwidth(vec *rtn, vec *args, win *w);
void indent_cmd_tabwidth(vec *rtn, vec *args, win *w);
void indent_cmd_indentmode(vec *rtn, vec *args, win *w);
void indent_cmd_autoindent(vec *rtn, vec *args, win *w);
void indent_cmd_incrindent(vec *rtn, vec *args, win *w);
void indent_cmd_decrindent(vec *rtn, vec *args, win *w);
void indent_cmd_indent(vec *rtn, vec *args, win *w);
#endif
