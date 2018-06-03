#include "bind.h"

#include "bind/mov.h"

table bind_mov;

BIND_FUNCT(mov_to_buf, bind_mode = bind_mode_buf)

void bind_mov_init(void)
{
    table *tab = &bind_mov;
    table_init(tab, sizeof(bind_info), sizeof(inp_key));

    BIND_TO(mov_to_buf, inp_key_ctrl | 'A', Switch to buffer mode);
}

void bind_mov_kill(void)
{
    table_kill(&bind_mov);
}