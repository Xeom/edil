#include <string.h>

#include "bind.h"
#include "cur.h"

#include "bind/curbind.h"

static int cur_speed = 0;
static int cur_speed_vals[] = { 5, 10, 20, 50 };

/* Move cursor */
BIND_FUNCT(cur_mv_u, cur_move_win(w, (cur){ .ln = -1 }))
BIND_FUNCT(cur_mv_d, cur_move_win(w, (cur){ .ln =  1 }))
BIND_FUNCT(cur_mv_l, cur_move_win(w, (cur){ .cn = -1 }))
BIND_FUNCT(cur_mv_r, cur_move_win(w, (cur){ .cn =  1 }))

BIND_FUNCT(cur_skp_u, cur_move_win(w, (cur){ .ln = -cur_speed_vals[cur_speed] }))
BIND_FUNCT(cur_skp_d, cur_move_win(w, (cur){ .ln =  cur_speed_vals[cur_speed] }))
BIND_FUNCT(cur_skp_l, cur_move_win(w, (cur){ .cn = -cur_speed_vals[cur_speed] }))
BIND_FUNCT(cur_skp_r, cur_move_win(w, (cur){ .cn =  cur_speed_vals[cur_speed] }))

BIND_FUNCT(cur_faster, if (cur_speed < 3) ++cur_speed;)
BIND_FUNCT(cur_slower, if (cur_speed > 0) --cur_speed;)

/* Move region */
BIND_FUNCT(cur_rmv_u, cur_shift(w, (cur){ .ln = -1 }))
BIND_FUNCT(cur_rmv_d, cur_shift(w, (cur){ .ln =  1 }))
BIND_FUNCT(cur_rmv_l, cur_shift(w, (cur){ .cn = -1 }))
BIND_FUNCT(cur_rmv_r, cur_shift(w, (cur){ .cn =  1 }))

/* Move cursor using navigation keys */
BIND_FUNCT(cur_home, cur_home_win(w))
BIND_FUNCT(cur_end,  cur_end_win(w))
BIND_FUNCT(cur_pgup, cur_pgup_win(w))
BIND_FUNCT(cur_pgdn, cur_pgdn_win(w))

BIND_FUNCT(cur_enter, cur_enter_win(w))
BIND_FUNCT(cur_del,   cur_del_win(w))
BIND_FUNCT(cur_back,
    if (memcmp(&(w->pri), &(cur){0, 0}, sizeof(cur)) != 0)
    {
        cur_move_win(w, (cur){ .cn = -1 });
        cur_del_win(w);
    }
)
BIND_FUNCT(cur_enter_line, cur_enter_line_win(w))

BIND_FUNCT(cur_ins_tab,
    vec tabvec;
    vec_init(&tabvec, sizeof(chr));
    vec_app(&tabvec, &CHR("\t"));

    cur_ins_win(w, &tabvec);

    vec_kill(&tabvec);
)

void bind_curbind_init(void)
{
    BIND_ADD(cur_mv_u, Move the cursor up);
    BIND_ADD(cur_mv_d, Move the cursor down);
    BIND_ADD(cur_mv_l, Move the cursor left);
    BIND_ADD(cur_mv_r, Move the cursor right);

    BIND_ADD(cur_skp_u, Skip the cursor up quickly);
    BIND_ADD(cur_skp_d, Skip the cursor down quickly);
    BIND_ADD(cur_skp_l, Skip the cursor left quickly);
    BIND_ADD(cur_skp_r, Skip the cursor right quickly);

    BIND_ADD(cur_faster, Increase the skip speed);
    BIND_ADD(cur_slower, Decrease the skip speed);

    BIND_ADD(cur_rmv_u, Shift the current region up);
    BIND_ADD(cur_rmv_d, Shift the current region down);
    BIND_ADD(cur_rmv_l, Shift the current region left);
    BIND_ADD(cur_rmv_r, Shift the current region right);

    BIND_ADD(cur_home, Move the cursor to the beginning of the line);
    BIND_ADD(cur_end,  Move the cursor to the end of the line);
    BIND_ADD(cur_pgup, Move the cursor to the top of the screen);
    BIND_ADD(cur_pgdn, Move the cursor to the bottom of the screen);

    BIND_ADD(cur_enter, Insert a newline at the current ursor);
    BIND_ADD(cur_enter_line, Insert a newline before the current line);

    BIND_ADD(cur_del,  Delete forwards at the cursor position);
    BIND_ADD(cur_back, Delete backwards at the cursor position);

    BIND_ADD(cur_ins_tab, Insert a tab at the cursor position);
}
