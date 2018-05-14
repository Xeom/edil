#if !defined(BIND_KCD_H)
# define BIND_KCD_H
# include "win.h"
# include "vec.h"

extern table bind_kcd;

void bind_kcd_init(void);
void bind_kcd_kill(void);

void bind_kcd_ins(win *w, vec *text);

#endif