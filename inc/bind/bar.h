#if !defined(BIND_BAR_H)
# define BIND_BAR_H
# include "win.h"
# include "vec.h"
# include "table.h"

extern table bind_bar;

void bind_bar_init(table *tab);

void bind_bar_ins(win *w, vec *text);

#endif
