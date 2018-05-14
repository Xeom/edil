#if !defined(BIND_BUF_H)
# define BIND_BUF_H
# include "win.h"
# include "vec.h"
# include "table.h"

extern table bind_buf;

void bind_buf_init(void);
void bind_buf_kill(void);

void bind_buf_ins(win *w, vec *text);

#endif