#include "bind.h"

#include "bind/bar.h"

table bind_bar;

BIND_FUNCT(bar_mv_l, bar_move(&(w->basebar), -1))
BIND_FUNCT(bar_mv_r, bar_move(&(w->basebar),  1))

BIND_FUNCT(bar_enter, bar_run(&(w->basebar)); bind_mode = bind_mode_buf)

BIND_FUNCT(bar_back, bar_back(&(w->basebar)))
BIND_FUNCT(bar_del,  bar_del(&(w->basebar)))

BIND_FUNCT(bar_to_buf,
    bind_mode = bind_mode_buf;
    bar_cancel(&(w->basebar));
)

BIND_FUNCT(bar_to_kcd,
    bind_mode = bind_mode_kcd;
    bar_cancel(&(w->basebar));
)

void bind_bar_init(table *tab)
{
    BIND_TO(bar_mv_l,  inp_key_left,  Move cursor left);
    BIND_TO(bar_mv_r,  inp_key_right, Move cursor right);
    BIND_TO(bar_enter, inp_key_enter, Submit contents of bar);
    BIND_TO(bar_back,  inp_key_back,  Delete backward);
    BIND_TO(bar_del,   inp_key_del,   Delete forward);

    BIND_TO(bar_to_buf, inp_key_ctrl | 'A', Switch to buffer mode);
    BIND_TO(bar_to_buf, inp_key_ctrl | 'X', Switch to buffer mode);
    BIND_TO(bar_to_kcd, inp_key_ctrl | 'K', Switch to keycode mode);
}

void bind_bar_ins(win *w, vec *text)
{
    bar_ins(&(w->basebar), text);
}
