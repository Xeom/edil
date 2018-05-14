#include <string.h>

#include "bind.h"
#include "ui.h"

#include "bind/buf.h"

table bind_buf;

static void bind_buf_shortcut_run(char *str, win *w);
static void bind_buf_shortcut(char *str, win *w);
static void bind_buf_activate_cmd(win *w);

/* Move cursor */
BIND_FUNCT(buf_mv_u, cur_move(w, (cur){ .ln = -1 }))
BIND_FUNCT(buf_mv_d, cur_move(w, (cur){ .ln =  1 }))
BIND_FUNCT(buf_mv_l, cur_move(w, (cur){ .cn = -1 }))
BIND_FUNCT(buf_mv_r, cur_move(w, (cur){ .cn =  1 }))

/* Move region */
BIND_FUNCT(buf_rmv_u, cur_move_region(w, (cur){ .ln = -1 }))
BIND_FUNCT(buf_rmv_d, cur_move_region(w, (cur){ .ln =  1 }))
BIND_FUNCT(buf_rmv_l, cur_move_region(w, (cur){ .cn = -1 }))
BIND_FUNCT(buf_rmv_r, cur_move_region(w, (cur){ .cn =  1 }))

/* Move cursor using navigation keys */
BIND_FUNCT(buf_home, cur_home(w))
BIND_FUNCT(buf_end,  cur_end(w))
BIND_FUNCT(buf_pgup, cur_pgup(w))
BIND_FUNCT(buf_pgdn, cur_pgdn(w))

BIND_FUNCT(buf_line, cur_lineify(w))

BIND_FUNCT(buf_enter, cur_enter(w))
BIND_FUNCT(buf_del,   cur_del(w))
BIND_FUNCT(buf_back,
    if (memcmp(&(w->pri), &(cur){0, 0}, sizeof(cur)) != 0)
    {
        cur_move(w, (cur){ .cn = -1 });
        cur_del(w);
    }
)

BIND_FUNCT(buf_ins_tab,
    vec tabvec;
    vec_init(&tabvec, sizeof(chr));
    vec_app(&tabvec, &CHR("\t"));

    cur_ins(w, &tabvec);

    vec_kill(&tabvec);
)

#define SCUT(cmd) \
    BIND_FUNCT(buf_ ## cmd ## _cmd, bind_buf_shortcut(#cmd " ", w))
#define RCUT(cmd) \
    BIND_FUNCT(buf_ ## cmd ## _cmd, bind_buf_shortcut_run(#cmd, w))

#define SCUT_TO(cmd, key) \
    BIND_TO(buf_ ## cmd ## _cmd, key)

SCUT(new) SCUT(load) SCUT(save) SCUT(goto) SCUT(quit)

RCUT(swap) RCUT(snap)
RCUT(next) RCUT(prev)
RCUT(copy) RCUT(paste)
RCUT(incrindent) RCUT(decrindent)

BIND_FUNCT(buf_to_kcd, bind_mode = bind_mode_kcd)
BIND_FUNCT(buf_to_cmd, bind_buf_activate_cmd(w))

void bind_buf_init(void)
{
    table *tab = &bind_buf;
    table_init(tab, sizeof(bind_fptr), sizeof(inp_key));

    BIND_TO(buf_mv_u, inp_key_up);
    BIND_TO(buf_mv_d, inp_key_down);
    BIND_TO(buf_mv_l, inp_key_left);
    BIND_TO(buf_mv_r, inp_key_right);

    BIND_TO(buf_rmv_u, inp_key_up    | inp_key_esc);
    BIND_TO(buf_rmv_d, inp_key_down  | inp_key_esc);
    BIND_TO(buf_rmv_l, inp_key_left  | inp_key_esc);
    BIND_TO(buf_rmv_r, inp_key_right | inp_key_esc);

    BIND_TO(buf_home, inp_key_home);
    BIND_TO(buf_end,  inp_key_end);
    BIND_TO(buf_pgup, inp_key_pgup);
    BIND_TO(buf_pgdn, inp_key_pgdn);

    BIND_TO(buf_line, inp_key_ctrl | 'L');

    BIND_TO(buf_enter, inp_key_enter);
    BIND_TO(buf_del,   inp_key_del);
    BIND_TO(buf_back,  inp_key_back);

    BIND_TO(buf_ins_tab, inp_key_tab);

    SCUT_TO(new,  inp_key_esc | 'n');
    SCUT_TO(load, inp_key_esc | 'a');
    SCUT_TO(save, inp_key_esc | 's');
    SCUT_TO(goto, inp_key_esc | 'g');
    SCUT_TO(quit, inp_key_esc | inp_key_ctrl | 'K');

    SCUT_TO(swap,  inp_key_ctrl | 'C');
    SCUT_TO(snap,  inp_key_ctrl | 'Z');
    SCUT_TO(next,  inp_key_ctrl | 'N');
    SCUT_TO(prev,  inp_key_ctrl | 'V');
    SCUT_TO(copy,  inp_key_ctrl | 'Y');
    SCUT_TO(paste, inp_key_ctrl | 'P');
    SCUT_TO(incrindent, inp_key_tab);
    SCUT_TO(decrindent, inp_key_esc | '[');

    BIND_TO(buf_to_kcd, inp_key_ctrl | 'K');
    BIND_TO(buf_to_cmd, inp_key_ctrl | 'X');
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
    cur_ins(w, text);
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
