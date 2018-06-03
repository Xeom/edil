#include "bind.h"

#include "bind/mov.h"

table bind_mov;

int mov_speed = 0;
int mov_speed_vals[] = { 5, 10, 20, 50 };

BIND_FUNCT(mov_to_buf, bind_mode = bind_mode_buf)
BIND_FUNCT(mov_mv_u, cur_move_win(w, (cur){ .ln = -1 }))
BIND_FUNCT(mov_mv_d, cur_move_win(w, (cur){ .ln =  1 }))
BIND_FUNCT(mov_mv_l, cur_move_win(w, (cur){ .cn = -1 }))
BIND_FUNCT(mov_mv_r, cur_move_win(w, (cur){ .cn =  1 }))

BIND_FUNCT(mov_skip_u, cur_move_win(w, (cur){ .ln = -mov_speed_vals[mov_speed] }))
BIND_FUNCT(mov_skip_d, cur_move_win(w, (cur){ .ln =  mov_speed_vals[mov_speed] }))
BIND_FUNCT(mov_skip_l, cur_move_win(w, (cur){ .cn = -mov_speed_vals[mov_speed] }))
BIND_FUNCT(mov_skip_r, cur_move_win(w, (cur){ .cn =  mov_speed_vals[mov_speed] }))

BIND_FUNCT(mov_faster, if (mov_speed < 3) ++mov_speed;)
BIND_FUNCT(mov_slower, if (mov_speed > 0) --mov_speed;)

BIND_FUNCT(mov_home, cur_home_win(w))
BIND_FUNCT(mov_end,  cur_end_win(w))
BIND_FUNCT(mov_pgup, cur_pgup_win(w))
BIND_FUNCT(mov_pgdn, cur_pgdn_win(w))

void bind_mov_init(table *tab)
{
    BIND_TO(mov_to_buf, inp_key_ctrl | 'A', Switch to buffer mode);

    BIND_TO(mov_mv_u, 'u', Move cursor up);
    BIND_TO(mov_mv_d, 'j', Move cursor down);
    BIND_TO(mov_mv_l, 'h', Move cursor left);
    BIND_TO(mov_mv_r, 'k', Move cursor right);

    BIND_TO(mov_skip_u, 'w', Skip cursor up);
    BIND_TO(mov_skip_d, 's', Skip cursor down);
    BIND_TO(mov_skip_l, 'a', Skip cursor left);
    BIND_TO(mov_skip_r, 'd', Skip cursor right);

    BIND_TO(mov_skip_u, inp_key_up, Skip cursor up);
    BIND_TO(mov_skip_d, inp_key_down, Skip cursor down);
    BIND_TO(mov_skip_l, inp_key_left, Skip cursor left);
    BIND_TO(mov_skip_r, inp_key_right, Skip cursor right);

    BIND_TO(mov_faster, 'e', Increase skip distance);
    BIND_TO(mov_slower, 'q', Decrease skip distance);

    BIND_TO(mov_pgup, 'f', Page up);
    BIND_TO(mov_pgdn, 'g', Page down);
    BIND_TO(mov_home, 'y', Home);
    BIND_TO(mov_end,  'i', End);
}
