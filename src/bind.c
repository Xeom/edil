#include <string.h>

#include "inp.h"
#include "win.h"
#include "ui.h"
#include "chr.h"

#include "bind/bar.h"
#include "bind/buf.h"
#include "bind/kcd.h"

#include "bind.h"

table bind_kcd;

bind_mode_type bind_mode;

static void bind_ins(inp_key key);

#define IS_TYPABLE(key) (key < 0x100 && key != inp_key_back)

vec bind_ins_buf;

void bind_init(void)
{
    bind_bar_init();
    bind_buf_init();
    bind_kcd_init();

    vec_init(&bind_ins_buf, sizeof(chr));
}

void bind_kill(void)
{
    bind_bar_kill();
    bind_buf_kill();
    bind_kcd_kill();

    vec_kill(&bind_ins_buf);
}

void bind_handle_key(inp_key key)
{
    bind_info *info;
    table *bindings;

    if (IS_TYPABLE(key) &&
            (bind_mode == bind_mode_buf || bind_mode == bind_mode_bar))
    {
        bind_ins(key);
        return;
    }
    else if (bind_mode == bind_mode_kcd && key != (inp_key_ctrl | 'A'))
    {
        bind_kcd_key(key, win_cur);
    }
    else
    {
        bind_ins_flush();
    }

    switch (bind_mode)
    {
    case bind_mode_buf: bindings = &bind_buf; break;
    case bind_mode_kcd: bindings = &bind_kcd; break;
    case bind_mode_bar: bindings = &bind_bar; break;
    }

    info = table_get(bindings, &key);

    if (!info) return;

    (*info->fptr)(key, win_cur);
    win_show_cur(win_cur, win_cur->pri);
}

static void bind_ins(inp_key key)
{
    static chr c = { .fnt = { .fg = col_none, .bg = col_none, .attr = 0 } };
    static int utf8ind = 0;
    static int width;

    c.utf8[utf8ind] = (char)(key & 0xff);

    if (utf8ind == 0)
    {
        width = chr_len(&c);
        memset(c.utf8 + 1, 0, sizeof(c.utf8) - 1);
    }

    if (++utf8ind == width)
    {
        utf8ind = 0;
        vec_app(&bind_ins_buf, &c);
    }
}

void bind_ins_flush(void)
{
    win *w;
    w = win_cur;

    if (vec_len(&bind_ins_buf) == 0)
        return;

    switch (bind_mode)
    {
    case bind_mode_buf: bind_buf_ins(w, &bind_ins_buf); break;
    case bind_mode_bar: bind_bar_ins(w, &bind_ins_buf); break;
    }

    vec_clr(&bind_ins_buf);
}

void bind_flush(void)
{
    bind_ins_flush();
    bar_out(&(win_cur->basebar));
}

#define BIND_PRINT_HEADER "\n"\
    " | Key             | Binding                                       |\n" \
    " |-----------------|-----------------------------------------------|\n"

void bind_print(FILE *stream)
{
    fputs("\nBuffer mode\n-----------" BIND_PRINT_HEADER, stream);
    bind_print_table(&bind_buf, stream);
    fputs("\nKeycode mode\n------------" BIND_PRINT_HEADER, stream);
    bind_print_table(&bind_kcd, stream);
    fputs("\nBar mode\n--------" BIND_PRINT_HEADER, stream);
    bind_print_table(&bind_bar, stream);
}

void bind_print_table(table *t, FILE *stream)
{
    vec  keys;
    bind_info *val = NULL;
    inp_key   *key;
    size_t     ind, len;

    vec_init(&keys, sizeof(inp_key));

    while ((val = table_next(t, val, (void *)&key)))
        vec_app(&keys, key);

    vec_sort(&keys, inp_key_cmp);

    len = vec_len(&keys);
    for (ind = 0; ind < len; ++ind)
    {
        char buf[64];
        key = vec_get(&keys, ind);
        val = table_get(t, key);
        if (!val) return;
        inp_key_name(*key, buf, 64);
        fprintf(stream, " | %-16s| %-45s |\n", buf, val->desc);
    }
}