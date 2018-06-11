#include "bind.h"
#include "bar.h"

#include "bind/barmap.h"


void bind_barmap_init()
{
    BIND_MAP_UDLR(bar, bar_mv, BIND_KEYS_UDLR(0));

    BIND_MAP(bar, bar_enter, inp_key_enter);
    BIND_MAP(bar, bar_back,  inp_key_back);
    BIND_MAP(bar, bar_del,   inp_key_del);

    BIND_MAP(bar, mode_buf,  inp_key_ctrl | 'X');
    BIND_MAP(bar, mode_buf,  inp_key_ctrl | 'A');
    BIND_MAP(bar, mode_kcd,  inp_key_ctrl | 'K');
}

void bind_bar_ins(win *w, vec *text)
{
    bar_ins(&(w->basebar), text);
}