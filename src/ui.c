#include <string.h>

#include "cur.h"
#include "win.h"
#include "chr.h"
#include "cmd.h"
#include "out.h"

#include "ui.h"

static void ui_handle_buf(inp_key key);
static void ui_handle_kcd(inp_key key);
static void ui_handle_bar(inp_key key);
static void ui_cmd_cb(win *w, vec *chrs);

ui_mode_type ui_mode = ui_mode_buf;
int ui_alive = 1;
char *ui_cmd_prompt = "$ ";
vec ui_ins_buf;

static void ui_cmd_cb(win *w, vec *chrs)
{
    vec args, rtn;
    size_t    argind;

    vec_init(&args, sizeof(vec));
    vec_init(&rtn,  sizeof(chr));

    cmd_parse(&args, chrs, 0);
    cmd_run(&args, &rtn, w);

    out_log(&rtn, stdout);

    for (argind = 0; argind < vec_len(&args); ++argind)
        vec_kill(vec_get(&args, argind));

    vec_kill(&args);
    vec_kill(&rtn);
}

void ui_init(void)
{
    vec_init(&ui_ins_buf, sizeof(chr));
}

void ui_kill(void)
{
    vec_kill(&ui_ins_buf);
}

void ui_ins_flush(void)
{
    win *w;
    size_t len;

    len = vec_len(&ui_ins_buf);

    if (len == 0)
        return;

    w = win_cur;

    switch (ui_mode)
    {
        case ui_mode_buf: w->pri = cur_ins(w->pri, w->b, &ui_ins_buf); break;
        case ui_mode_bar: win_bar_ins(w, &ui_ins_buf); break;
    }

    vec_del(&ui_ins_buf, 0, len);
}

void ui_flush(void)
{
    ui_ins_flush();

    win_out_after(win_cur, (cur){0, 0}, stdout);
    win_out_bar(win_cur, stdout);
}

int ui_is_typable(inp_key key)
{
    return (key < 0x100 && key != inp_key_back);
}

void ui_ins(inp_key key)
{
    static chr c = { .fnt = { .fg = col_none, .bg = col_none } };
    static int utf8ind = 0, width;

    c.utf8[utf8ind] = (char)(key & 0xff);

    if (utf8ind == 0)
    {
        width = chr_len(&c);
        memset(c.utf8 + 1, 0, sizeof(c.utf8) - 1);
    }
    if (++utf8ind == width)
    {
        utf8ind = 0;
        vec_ins(&ui_ins_buf, vec_len(&ui_ins_buf), 1, &c);
    }
}

void ui_handle(inp_key key)
{
    vec cmdprompt;
    int modechanged;
    modechanged = 1;

    if (!ui_is_typable(key))
    {
        ui_ins_flush();
    }

    switch (key)
    {
    case inp_key_ctrl | 'X': ui_mode = ui_mode_bar;
        vec_init(&cmdprompt, sizeof(chr));
        chr_from_str(&cmdprompt, ui_cmd_prompt);
        win_bar_query(win_cur, &cmdprompt, ui_cmd_cb);
        vec_kill(&cmdprompt);
        break;

    case inp_key_ctrl | 'K': ui_mode = ui_mode_kcd; break;
    case inp_key_ctrl | 'A': ui_mode = ui_mode_buf; break;
    case inp_key_ctrl | inp_key_esc | 'K': ui_alive = 0; break;
    default: modechanged = 0;
    }

    if (modechanged) return;

    switch (ui_mode)
    {
    case ui_mode_buf: ui_handle_buf(key); break;
    case ui_mode_kcd: ui_handle_kcd(key); break;
    case ui_mode_bar: ui_handle_bar(key); break;
    }
}

static void ui_handle_kcd(inp_key key)
{
    char buf[32];
    vec chrs;
    win *w;
    w = win_cur;

    vec_init(&chrs, sizeof(chr));

    inp_key_name(key, buf, sizeof(buf));

    chr_from_str(&chrs, buf);

    w->pri = cur_ins(w->pri, w->b, &chrs);

    vec_kill(&chrs);
}

void ui_handle_buf(inp_key key)
{
    win *w;
    w = win_cur;

    if (ui_is_typable(key))
    {
        ui_ins(key);
    }

    switch (key)
    {
    case inp_key_enter: w->pri = cur_enter(w->pri, w->b); break;

    case inp_key_up:    w->pri = cur_move(w->pri, w->b, (cur){ .ln = -1 }); break;
    case inp_key_down:  w->pri = cur_move(w->pri, w->b, (cur){ .ln =  1 }); break;
    case inp_key_left:  w->pri = cur_move(w->pri, w->b, (cur){ .cn = -1 }); break;
    case inp_key_right: w->pri = cur_move(w->pri, w->b, (cur){ .cn =  1 }); break;

    case inp_key_home:  w->pri = cur_home(w->pri, w->b); break;
    case inp_key_end:   w->pri = cur_end (w->pri, w->b); break;

    case inp_key_back:  w->pri = cur_move(w->pri, w->b, (cur){ .cn = -1 });
    case inp_key_del:   w->pri = cur_del (w->pri, w->b);  break;
    }

    win_show_cur(w, w->pri, stdout);
}

static void ui_handle_bar(inp_key key)
{
    win *w;
    w = win_cur;

    if (ui_is_typable(key))
    {
        ui_ins(key);
    }

    switch (key)
    {
    case inp_key_enter: win_bar_run(w); ui_mode = ui_mode_buf; break;

    case inp_key_left:  win_bar_move(w, -1); break;
    case inp_key_right: win_bar_move(w,  1); break;

    case inp_key_back:  win_bar_back(w); break;
    case inp_key_del:   win_bar_del(w);  break;
    }
}
