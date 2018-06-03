#include <string.h>

#include "bind.h"
#include "ui.h"

#include "bind/buf.h"

table bind_buf;

static void bind_buf_shortcut_run(char *str, win *w);
static void bind_buf_shortcut(char *str, win *w);
static void bind_buf_activate_cmd(win *w);

/* Move cursor */
BIND_FUNCT(buf_mv_u, cur_move_win(w, (cur){ .ln = -1 }))
BIND_FUNCT(buf_mv_d, cur_move_win(w, (cur){ .ln =  1 }))
BIND_FUNCT(buf_mv_l, cur_move_win(w, (cur){ .cn = -1 }))
BIND_FUNCT(buf_mv_r, cur_move_win(w, (cur){ .cn =  1 }))

/* Move region */
BIND_FUNCT(buf_rmv_u, cur_shift(w, (cur){ .ln = -1 }))
BIND_FUNCT(buf_rmv_d, cur_shift(w, (cur){ .ln =  1 }))
BIND_FUNCT(buf_rmv_l, cur_shift(w, (cur){ .cn = -1 }))
BIND_FUNCT(buf_rmv_r, cur_shift(w, (cur){ .cn =  1 }))

/* Move cursor using navigation keys */
BIND_FUNCT(buf_home, cur_home_win(w))
BIND_FUNCT(buf_end,  cur_end_win(w))
BIND_FUNCT(buf_pgup, cur_pgup_win(w))
BIND_FUNCT(buf_pgdn, cur_pgdn_win(w))

BIND_FUNCT(buf_line, cur_lineify_win(w))

BIND_FUNCT(buf_enter, cur_enter_win(w))
BIND_FUNCT(buf_del,   cur_del_win(w))
BIND_FUNCT(buf_back,
    if (memcmp(&(w->pri), &(cur){0, 0}, sizeof(cur)) != 0)
    {
        cur_move_win(w, (cur){ .cn = -1 });
        cur_del_win(w);
    }
)
BIND_FUNCT(buf_enter_line, cur_enter_line_win(w))

BIND_FUNCT(buf_ins_tab,
    vec tabvec;
    vec_init(&tabvec, sizeof(chr));
    vec_app(&tabvec, &CHR("\t"));

    cur_ins_win(w, &tabvec);

    vec_kill(&tabvec);
)

#define SCUT(cmd) \
    BIND_FUNCT(buf_ ## cmd ## _cmd, bind_buf_shortcut(#cmd " ", w))
#define RCUT(cmd) \
    BIND_FUNCT(buf_ ## cmd ## _cmd, bind_buf_shortcut_run(#cmd, w))

#define SCUT_TO(cmd, key, desc) \
    BIND_TO(buf_ ## cmd ## _cmd, key, desc)

SCUT(new) SCUT(load) SCUT(save) SCUT(goto) SCUT(quit)

RCUT(swap) RCUT(snap)
RCUT(next) RCUT(prev)
RCUT(copy) RCUT(paste)
RCUT(incrindent) RCUT(decrindent)

BIND_FUNCT(buf_to_kcd, bind_mode = bind_mode_kcd)
BIND_FUNCT(buf_to_cmd, bind_buf_activate_cmd(w))
BIND_FUNCT(buf_to_mov, bind_mode = bind_mode_mov)

void bind_buf_init(void)
{
    table *tab = &bind_buf;
    table_init(tab, sizeof(bind_info), sizeof(inp_key));

    BIND_TO(buf_mv_u, inp_key_up,    Move the cursor up);
    BIND_TO(buf_mv_d, inp_key_down,  Move the cursor down);
    BIND_TO(buf_mv_l, inp_key_left,  Move the cursor left);
    BIND_TO(buf_mv_r, inp_key_right, Move the cursor right);

    BIND_TO(buf_rmv_u, inp_key_up    | inp_key_esc, Shift the current region up);
    BIND_TO(buf_rmv_d, inp_key_down  | inp_key_esc, Shift the current region down);
    BIND_TO(buf_rmv_l, inp_key_left  | inp_key_esc, Shift the current region left);
    BIND_TO(buf_rmv_r, inp_key_right | inp_key_esc, Shift the current region right);

    BIND_TO(buf_home, inp_key_home, Move the cursor to the beginning of the line);
    BIND_TO(buf_end,  inp_key_end,  Move the cursor to the end of the line);
    BIND_TO(buf_pgup, inp_key_pgup, Move the cursor to the top of the screen);
    BIND_TO(buf_pgdn, inp_key_pgdn, Move the cursor to the bottom of the screen);

    BIND_TO(buf_line, inp_key_ctrl | 'L', Select the current line as the region);

    BIND_TO(buf_enter, inp_key_enter, Insert a newline);
    BIND_TO(buf_del,   inp_key_del,   Delete a character forward);
    BIND_TO(buf_back,  inp_key_back,  Delete a character backward);

    BIND_TO(buf_enter_line, inp_key_enter | inp_key_esc, Insert a newline before current line);

    BIND_TO(buf_ins_tab, inp_key_tab | inp_key_esc, Insert a tab character);

    SCUT_TO(new,  inp_key_esc | 'n', Shortcut for 'new' command);
    SCUT_TO(load, inp_key_esc | 'a', Shortcut for 'load' command);
    SCUT_TO(save, inp_key_esc | 's', Shortcut for 'save' command);
    SCUT_TO(goto, inp_key_esc | 'g', Shortcut for 'goto' command);

    SCUT_TO(quit, inp_key_esc | inp_key_ctrl | 'K', Shortcut for 'quit' command);

    SCUT_TO(swap,  inp_key_ctrl | 'C', Run 'swap' command);
    SCUT_TO(snap,  inp_key_ctrl | 'Z', Run 'snap' command);
    SCUT_TO(next,  inp_key_ctrl | 'N', Run 'next' command);
    SCUT_TO(prev,  inp_key_ctrl | 'V', Run 'prev' command);
    SCUT_TO(copy,  inp_key_ctrl | 'Y', Run 'copy' command);
    SCUT_TO(paste, inp_key_ctrl | 'P', Run 'paste' command);

    SCUT_TO(incrindent, inp_key_tab,      Run 'incrindent' command);
    SCUT_TO(decrindent, inp_key_shifttab, Run 'decrindent' command);

    BIND_TO(buf_to_kcd, inp_key_ctrl | 'K', Switch to keycode mode);
    BIND_TO(buf_to_cmd, inp_key_ctrl | 'X', Switch to command mode);
    BIND_TO(buf_to_mov, inp_key_ctrl | 'A', Switch to movement mode);
}

void bind_buf_kill(void)
{
    table_kill(&bind_buf);
}

/* Command shortcuts */
static void bind_buf_shortcut(char *str, win *w)
{
    vec chrs;
    vec_init(&chrs, sizeof(chr));
    chr_from_str(&chrs, str);

    bind_buf_activate_cmd(w);
    bar_ins(&(w->basebar), &chrs);

    vec_kill(&chrs);
}

static void bind_buf_shortcut_run(char *str, win *w)
{
    vec chrs;
    vec_init(&chrs, sizeof(chr));
    chr_from_str(&chrs, str);

    ui_cmd_cb(w, &chrs);

    vec_kill(&chrs);
}

void bind_buf_ins(win *w, vec *text)
{
    cur_ins_win(w, text);
}

void bind_buf_activate_cmd(win *w)
{
    vec cmdprompt;
    vec_init(&cmdprompt, sizeof(chr));
    chr_from_str(&cmdprompt, "$ ");//ui_cmd_prompt);

    bind_mode = bind_mode_bar;

    bar_query(&(w->basebar), &cmdprompt, ui_cmd_cb);
    vec_kill(&cmdprompt);
}

#pragma GCC diagnostic pop
