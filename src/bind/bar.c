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

void bind_bar_init(void)
{
    table *tab = &bind_bar;
    table_init(tab, sizeof(bind_fptr), sizeof(inp_key));

    BIND_TO(bar_mv_l,  inp_key_left);
    BIND_TO(bar_mv_r,  inp_key_right);
    BIND_TO(bar_enter, inp_key_enter);
    BIND_TO(bar_back,  inp_key_back);
    BIND_TO(bar_del,   inp_key_del);

    BIND_TO(bar_to_buf, inp_key_ctrl | 'A');
    BIND_TO(bar_to_buf, inp_key_ctrl | 'X');
    BIND_TO(bar_to_kcd, inp_key_ctrl | 'K');
}

void bind_bar_kill(void)
{
    table_kill(&bind_bar);
}

void bind_bar_ins(win *w, vec *text)
{
    bar_ins(&(w->basebar), text);
}
