#include "bind.h"

#include "bind/mov.h"

table bind_mov;

BIND_FUNCT(mov_to_buf, bind_mode = bind_mode_buf)

void bind_mov_init(table *tab)
{
    BIND_TO(mov_to_buf, inp_key_ctrl | 'A', Switch to buffer mode);
}
