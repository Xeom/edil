#if !defined(BIND_KCD_H)
# define BIND_KCD_H
# include "types.h"

extern table bind_kcd;

void bind_kcd_init(table *tab);

void bind_kcd_key(win *w, inp_key k);

#endif
