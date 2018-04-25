#if !defined(CMD_REGION)
# define CMD_REGION
# include "buf.h"
# include "vec.h"
# include "win.h"

extern buf *region_clipboard;

void region_cmd_copy(vec *rtn, vec *args, win *w);

void region_cmd_paste(vec *rtn, vec *args, win *w);

void region_cmd_cut(vec *rtn, vec *args, win *w);

#endif
