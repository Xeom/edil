#include <ctype.h>
#include <string.h>

#include "inp.h"
#include "win.h"
#include "ui.h"
#include "chr.h"

#include "bind/bar.h"
#include "bind/buf.h"
#include "bind/kcd.h"
#include "bind/mov.h"

#include "bind.h"

bind_mode_type bind_mode;
vec bind_all;

static void bind_ins(inp_key key);
static void bind_ins_flush(void);

#define IS_TYPABLE(key) (key < 0x100 && key != inp_key_back)

static vec bind_ins_buf;

bind_mode_info bind_modes[] =
{
    [bind_mode_buf] = {
        "buf", bind_mode_buf,
        &bind_buf,
        bind_buf_init,
        bind_buf_ins, NULL
    },

    [bind_mode_bar] = {
        "bar", bind_mode_bar,
        &bind_bar,
        bind_bar_init,
        bind_bar_ins, NULL
    },

    [bind_mode_kcd] = {
        "kcd", bind_mode_kcd,
        &bind_kcd,
        bind_kcd_init,
        NULL, bind_kcd_key
    },

    [bind_mode_mov] = {
        "mov", bind_mode_mov,
        &bind_mov,
        bind_mov_init,
        NULL, NULL
    }
};

#define FOREACH_MODE(info, code) \
    { \
        size_t _ind, _num; \
        _num = sizeof(bind_modes)/sizeof(bind_modes[0]); \
        for (_ind = 0; _ind < _num; ++_ind) \
        { \
            bind_mode_info *info = &bind_modes[_ind]; \
            {code} \
        } \
    }

#define GET_CURR_INFO(info) \
    bind_mode_info *info = &bind_modes[bind_mode];

bind_mode_type bind_mode_get(vec *chrname)
{
    vec str;
    vec_init(&str, sizeof(char));

    chr_to_str(chrname, &str);
    vec_app(&str, "\0");

    FOREACH_MODE(info,
        if (strcmp(info->name, vec_first(&str)) == 0)
        {
            vec_kill(&str);
            return info->mode;
        }
    );

    vec_kill(&str);

    return bind_mode_none;
}

bind_info *bind_info_get(vec *chrname)
{
    size_t n;
    namevec_item *item;
    item = namevec_get_chrs(&bind_all, chrname, &n);

    if (n == 1)
        return item->data.bind;
    else
        return NULL;
}

bind_mode_info *bind_info_curr(void)
{
    GET_CURR_INFO(rtn);
    return rtn;
}

int bind_remap(vec *chrmode, inp_key k, vec *chrbind)
{
    bind_mode_type  mode;
    bind_mode_info *modeinfo;
    bind_info      *bindinfo;

    mode = bind_mode_get(chrmode);

    if (mode == bind_mode_none) return -1;

    modeinfo = &bind_modes[mode];
    bindinfo = bind_info_get(chrbind);

    if (!bindinfo) return -1;

    table_set(modeinfo->keytable, &k, bindinfo);

    return 0;
}

int bind_unmap(vec *chrmode, inp_key k)
{
    bind_mode_type  mode;
    bind_mode_info *modeinfo;

    mode = bind_mode_get(chrmode);

    if (mode == bind_mode_none) return -1;

    modeinfo = &bind_modes[mode];

    table_delete(modeinfo->keytable, &k);

    return 0;
}

void bind_init(void)
{
    vec_init(&bind_all, sizeof(namevec_item));

    FOREACH_MODE(info,
        table *tab = info->keytable;
        table_init(tab, sizeof(bind_info), sizeof(inp_key));
        (info->initf)(tab);
    )

    namevec_sort(&bind_all);

    vec_init(&bind_ins_buf, sizeof(chr));
}

void bind_kill(void)
{
    FOREACH_MODE(info, table_kill(info->keytable););

    vec_kill(&bind_ins_buf);
}

void bind_handle_key(inp_key key)
{
    bind_info *bnd;

    GET_CURR_INFO(info)

    bnd = table_get(info->keytable, &key);

    if (bnd)
    {
        bind_ins_flush();

        (bnd->fptr)(win_cur, key);
        // TODO: move this somewhere nicer
        win_show_cur(win_cur, win_cur->pri);
    }
    else if (info->keyf)
    {
        (info->keyf)(win_cur, key);
        // TODO: move this somewhere nicer
        win_show_cur(win_cur, win_cur->pri);
    }
    else if (IS_TYPABLE(key) && info->insf)
    {
        bind_ins(key);
    }
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

static void bind_ins_flush(void)
{
    GET_CURR_INFO(info)

    if (vec_len(&bind_ins_buf) == 0)
        return;

    if (info->insf)
        (info->insf)(win_cur, &bind_ins_buf);

    vec_clr(&bind_ins_buf);
}

void bind_flush(void)
{
    bind_ins_flush();
    bar_out(&(win_cur->basebar));
}

#define BIND_PRINT_HEADER(info) "\n" \
    "%c%s mode\n" \
    "--------\n " \
    "| Key                 " \
    "| Bind name           " \
    "| Description                                   |\n " \
    "|---------------------" \
    "|---------------------" \
    "|-----------------------------------------------|\n", \
    toupper(info->name[0]), info->name + 1

void bind_print(FILE *stream)
{
    FOREACH_MODE(info,
        fprintf(stream, BIND_PRINT_HEADER(info));
        bind_print_table(info->keytable, stream);
    )
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
        fprintf(stream, " | %-20s| %-20s| %-45s |\n", buf, val->name, val->desc);
    }

    vec_kill(&keys);
}