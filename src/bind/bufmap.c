#include "text/cur.h"

#include "bind/curbind.h"
#include "bind/cmdbind.h"

#include "bind.h"
#include "bind/bufmap.h"

void bind_bufmap_init(void)
{
    BIND_MAP_UDLR(buf, cur_mv,  BIND_KEYS_UDLR(0));
    BIND_MAP_UDLR(buf, cur_rmv, BIND_KEYS_UDLR(inp_key_esc));

    BIND_MAP(buf, cur_home, inp_key_home);
    BIND_MAP(buf, cur_end,  inp_key_end);
    BIND_MAP(buf, cur_pgup, inp_key_pgup);
    BIND_MAP(buf, cur_pgdn, inp_key_pgdn);

    BIND_MAP(buf, cur_enter, inp_key_enter);
    BIND_MAP(buf, cur_enter_line, inp_key_esc | inp_key_enter);

    BIND_MAP(buf, cur_del,  inp_key_del);
    BIND_MAP(buf, cur_back, inp_key_back);

    BIND_MAP(buf, cur_ins_tab, inp_key_esc | inp_key_tab);

    BIND_MAP(buf, mode_cmd, inp_key_ctrl | 'X');
    BIND_MAP(buf, mode_kcd, inp_key_ctrl | '^');
    BIND_MAP(buf, mode_mov, inp_key_ctrl | 'E');
    BIND_MAP(buf, mode_mov, inp_key_esc  | 'm');
    BIND_MAP(buf, mode_lng, inp_key_ctrl | 'L');

    BIND_CMD_MAP_ALL(buf);
}

void bind_buf_ins(win *w, vec *text)
{
    cur_ins_win(w, text);
}
