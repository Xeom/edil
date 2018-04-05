#include <string.h>

#include "indent.h"
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
static void ui_handle_indent(inp_key key);
static void ui_handle_move(inp_key key);
static void ui_handle_ins(inp_key key);
static void ui_handle_shortcut(inp_key key);
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

    win_out_all();

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

    win_show_cur(w, w->pri);
    vec_del(&ui_ins_buf, 0, len);
}

void ui_flush(void)
{
    ui_ins_flush();

    win_out_bar(win_cur);
}

int ui_is_typable(inp_key key)
{
    return (key < 0x100 && key != inp_key_back);
}

void ui_activate_cmd(void)
{
    vec cmdprompt;
    ui_mode = ui_mode_bar;

    vec_init(&cmdprompt, sizeof(chr));
    chr_from_str(&cmdprompt, ui_cmd_prompt);
    win_bar_query(win_cur, &cmdprompt, ui_cmd_cb);
    vec_kill(&cmdprompt);
}

void ui_handle(inp_key key)
{
    int modechanged;
    modechanged = 1;

    if (!ui_is_typable(key))
    {
        ui_ins_flush();
    }

    switch (key)
    {
    case inp_key_ctrl | 'X': ui_activate_cmd();  break;
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

static void ui_handle_ins(inp_key key)
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

static void ui_handle_kcd(inp_key key)
{
    char buf[32];
    cur  prev;
    vec chrs;
    win *w;
    w = win_cur;
    prev = w->pri;

    vec_init(&chrs, sizeof(chr));

    inp_key_name(key, buf, sizeof(buf));

    chr_from_str(&chrs, buf);

    w->pri = cur_ins(w->pri, w->b, &chrs);

    win_out_line(w, prev);
    vec_kill(&chrs);
}

static void ui_handle_move(inp_key key)
{
    cur  c, prev;
    buf *b;
    win *w;
    w = win_cur;
    b = w->b;
    c = w->pri;

    switch(key)
    {
    case inp_key_up:
        c = cur_move(c, b, (cur){ .ln = -1 });
        break;
    case inp_key_down:
        c = cur_move(c, b, (cur){ .ln =  1 });
        break;
    case inp_key_left:
        c = cur_move(c, b, (cur){ .cn = -1 });
        break;
    case inp_key_right:
        c = cur_move(c, b, (cur){ .cn =  1 });
        break;

    case inp_key_home:
        c = cur_home(c, b);
        break;
    case inp_key_end:
        c = cur_end(c, b);
        break;
    case inp_key_pgdn:
        c = cur_pgdn(c, w);
        break;
    case inp_key_pgup:
        c = cur_pgup(c, w);
        break;
    }

    prev = w->pri;
    w->pri = c;

    if (prev.ln == w->pri.ln)
    {
        win_out_line(w, (w->pri.cn > prev.cn) ? prev : w->pri);
    }
    else
    {
        win_out_line(w, w->pri);
        win_out_line(w, prev);
    }
}

void ui_handle_buf(inp_key key)
{
    win *w;
    cur prev;
    w = win_cur;
    prev = w->pri;

    if (ui_is_typable(key))
    {
        ui_handle_ins(key);
    }
    else switch (key)
    {
    case inp_key_enter:
        w->pri = cur_enter(w->pri, w->b);
        win_out_after(w, prev);
        break;

    case inp_key_back:
        w->pri = cur_move(w->pri, w->b, (cur){ .cn = -1 });
    case inp_key_del:
        w->pri = cur_del (w->pri, w->b);

        if (prev.ln != w->pri.ln)
            win_out_after(w, w->pri);
        else
            win_out_line(w, w->pri);
        break;
    default:
        ui_handle_move(key);
        ui_handle_indent(key);
        ui_handle_shortcut(key);
    }

    win_show_cur(w, w->pri);
}

static void ui_handle_shortcut(inp_key key)
{
    win *w;
    w = win_cur;
    char *cmd;

    cmd = NULL;

    switch (key)
    {
    case 'n' | inp_key_esc: cmd = "new "; break;
    case 'a' | inp_key_esc: cmd = "associate "; break;
    case 's' | inp_key_esc: cmd = "save "; break;
    case 'g' | inp_key_esc: cmd = "goto "; break;
    }

    if (cmd)
    {
        ui_activate_cmd();
        chr_from_str(&(w->bartyped), cmd);
        w->barcur = vec_len(&(w->bartyped));

        return;
    }

    switch (key)
    {
    case 'C' | inp_key_ctrl: cmd = "swap"; break;
    case 'N' | inp_key_ctrl: cmd = "next"; break;
    case 'V' | inp_key_ctrl: cmd = "prev"; break;
    }

    if (cmd)
    {
        vec cmdchrs;
        vec_init(&cmdchrs, sizeof(chr));
        chr_from_str(&cmdchrs, cmd);
        ui_cmd_cb(w, &cmdchrs);
        vec_kill(&cmdchrs);
    }
}

static void ui_handle_indent(inp_key key)
{
    vec tabvec;
    win *w;
    buf *b;
    w = win_cur;
    b = w->b;

    switch (key)
    {
    case inp_key_tab:
        w->pri = indent_incr_depth(b, w->pri);
        break;
    case '[' | inp_key_esc: /* Shift + Tab */
        w->pri = indent_decr_depth(b, w->pri);
        break;
    case inp_key_tab | inp_key_esc:
        vec_init(&tabvec, sizeof(chr));
        vec_ins(&tabvec, 0, 1, &CHR("\t"));
        w->pri = cur_ins(w->pri, b, &tabvec);
        vec_kill(&tabvec);
        break;
    }

    win_out_line(w, (cur){ .ln = w->pri.ln });
}

static void ui_handle_bar(inp_key key)
{
    win *w;
    w = win_cur;

    if (ui_is_typable(key))
    {
        ui_handle_ins(key);
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
