#include "buf/cur.h"

#include "bind/curbind.h"
#include "bind/cmdbind.h"

#include "bind.h"
#include "bind/lngmap.h"

void bind_lngmap_init(void)
{
    BIND_MAP_UDLR(lng, cur_mv,  BIND_KEYS_UDLR(0));
    BIND_MAP_UDLR(lng, cur_rmv, BIND_KEYS_UDLR(inp_key_esc));

    BIND_MAP(lng, cur_home, inp_key_home);
    BIND_MAP(lng, cur_end,  inp_key_end);
    BIND_MAP(lng, cur_pgup, inp_key_pgup);
    BIND_MAP(lng, cur_pgdn, inp_key_pgdn);

    BIND_MAP(lng, cur_line, inp_key_ctrl | 'L');

    BIND_MAP(lng, cur_enter, inp_key_enter);
    BIND_MAP(lng, cur_enter_line, inp_key_esc | inp_key_enter);

    BIND_MAP(lng, cur_del_long,  inp_key_del);
    BIND_MAP(lng, cur_back_long, inp_key_back);

    BIND_MAP(lng, cur_ins_tab_long, inp_key_esc | inp_key_tab);

    BIND_MAP(lng, mode_cmd, inp_key_ctrl | 'X');
    BIND_MAP(lng, mode_kcd, inp_key_ctrl | '^');
    BIND_MAP(lng, mode_buf, inp_key_ctrl | 'E');
    BIND_MAP(lng, mode_buf, inp_key_ctrl | 'L');
    BIND_MAP(lng, mode_mov, inp_key_esc  | 'm');

    BIND_CMD_MAP_ALL(lng);

    bind_unmap_str(bind_mode_lng, inp_key_tab);
    bind_unmap_str(bind_mode_lng, inp_key_shifttab);
}

void bind_lng_ins(win *w, vec *text)
{
    cur_ins_long_win(w, text);
}
