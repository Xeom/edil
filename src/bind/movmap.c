#include "bind.h"

#include "bind/curbind.h"
#include "bind/cmdbind.h"

#include "bind/movmap.h"

void bind_movmap_init(void)
{
    BIND_MAP_UDLR(mov, cur_skp, 'w', 's', 'a', 'd');
    BIND_MAP_UDLR(mov, cur_mv,  'u', 'j', 'h', 'k');
    BIND_MAP_UDLR(mov, cur_skp, BIND_KEYS_UDLR(0));
    BIND_MAP_UDLR(mov, cur_mv,  BIND_KEYS_UDLR(inp_key_esc));

    BIND_MAP(mov, cur_faster, 'e');
    BIND_MAP(mov, cur_slower, 'q');

    BIND_MAP(mov, cur_pgup, 'f');
    BIND_MAP(mov, cur_pgdn, 'g');
    BIND_MAP(mov, cur_home, 'y');
    BIND_MAP(mov, cur_end,  'i');

    BIND_MAP(mov, mode_cmd, inp_key_ctrl | 'X');
    BIND_MAP(mov, mode_kcd, inp_key_ctrl | 'K');
    BIND_MAP(mov, mode_buf, inp_key_ctrl | 'A');
    BIND_MAP(mov, mode_cmd, ':');
    BIND_MAP(mov, mode_buf, ' ');

    BIND_CMD_MAP_ALL(mov);
}
