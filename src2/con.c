#include "cur.h"
#include "win.h"

#include "con.h"

void con_handle(inp_key key)
{
    win *w;
    w = win_cur;

    switch (key)
    {
    case inp_key_enter:
        w->pri = cur_enter(w->pri, w->b);
        break;
    case inp_key_up:
        w->pri = cur_move(w->pri, w->b, (cur){ .ln = -1 });
        break;
    case inp_key_down:
        w->pri = cur_move(w->pri, w->b, (cur){ .ln =  1 });
        break;
    case inp_key_left:
        w->pri = cur_move(w->pri, w->b, (cur){ .cn = -1 });
        break;
    case inp_key_right:
        w->pri = cur_move(w->pri, w->b, (cur){ .cn =  1 });
        break;
    }

    win_out_after(w, (cur){0, 0}, stdout);
}
