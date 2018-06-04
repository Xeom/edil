#include "bind.h"

#include "bind/barbind.h"

BIND_FUNCT(bar_mv_l, bar_move(&(w->basebar), -1))
BIND_FUNCT(bar_mv_r, bar_move(&(w->basebar),  1))

BIND_FUNCT(bar_enter,
    bar_run(&(w->basebar));
    bind_mode = bind_mode_buf
)

BIND_FUNCT(bar_back, bar_back(&(w->basebar)))
BIND_FUNCT(bar_del,  bar_del(&(w->basebar)))

void bind_barbind_init(void)
{
    BIND_ADD(bar_mv_l, Move bar cursor left);
    BIND_ADD(bar_mv_r, Move bar cursor right);

    BIND_ADD(bar_enter, Submit contents of bar);
    BIND_ADD(bar_back,  Delete backwards in bar);
    BIND_ADD(bar_del,   Delete forwards in bar);
}