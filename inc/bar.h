#if !defined(BAR_H)
# define BAR_H
# include "types.h"

void bar_init(bar *b, win *w);

void bar_kill(bar *b);

void bar_set_format(bar *b, char *fmt);

void bar_get_content(bar *b, vec *cont);

void bar_ins(bar *b, vec *chrs);

void bar_back(bar *b);

void bar_del(bar *b);

void bar_move(bar *b, int n);

void bar_run(bar *b);

void bar_cancel(bar *b);

void bar_query(bar *b, vec *prompt, void (*cb)(win *w, vec *chrs));

void bar_out(bar *b);

void bar_scrollback(bar *b, int dir);

void bar_scrollback_new(bar *b);

void bar_scrollback_reset(bar *b);

#endif
