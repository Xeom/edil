#include <string.h>
#include <stdlib.h>

#include "out.h"
#include "container/vec.h"
#include "text/chr.h"
#include "text/col.h"
#include "win.h"
#include "file.h"
#include "bind.h"
#include "container/circvec.h"
#include "text/buf.h"

#include "bar.h"

static int bar_append_format(bar *b, vec *cont, char **str);

chr bar_blank_chr = {
    .utf8 = "\xe2\x94\x80", .fnt = { .fg = col_black, .bg = col_black | col_bright }
};

col bar_cont_col = {
    .fg = col_black, .bg = col_white, .attr = 0
};

col_desc bar_prompt_col = {
    .inv = col_rev, .fg = col_null, .bg = col_black | col_bright
};

col_desc bar_typed_col = {
    .inv = col_rev, .fg = col_null, .bg = col_null
};

#define BAR_CONT_SEP "%8\xe2\x94\x82%0"

char *bar_format_default = " %n " BAR_CONT_SEP " %L\xc2\xb7%C " BAR_CONT_SEP " %m ";

void bar_init(bar *b, win *w)
{
    b->ind = 0;
    b->w   = w;
    b->cb  = NULL;

    circvec_init(&(b->scrollback), sizeof(vec), 5);
    bar_scrollback_new(b);
    vec_init(&(b->prompt), sizeof(chr));

    b->format = NULL;
    bar_set_format(b, bar_format_default);
}

void bar_kill(bar *b)
{
    vec_kill(&(b->prompt));

    while (!circvec_empty(&(b->scrollback)))
        vec_kill(circvec_pop(&(b->scrollback)));

    circvec_kill(&(b->scrollback));

    free(b->format);
}

/* Functions for generating the bar information */

void bar_set_format(bar *b, char *fmt)
{
    if (b->format)
        free(b->format);

    b->format = malloc(strlen(fmt) + 1);
    strcpy(b->format, fmt);
}

static int bar_append_format(bar *b, vec *cont, char **str)
{
    win *w;
    buf *bf;
    cur pri, sec;

    w  = b->w;
    bf = w->b;

    pri = w->pri;
    sec = w->sec;

    int percent;

    switch (**str)
    {
    case 'L': chr_format(cont, "%ld", pri.ln + 1);  break;
    case 'C': chr_format(cont, "%ld", pri.cn + 1);  break;
    case 'l': chr_format(cont, "%ld", sec.ln + 1);  break;
    case 'c': chr_format(cont, "%ld", sec.cn + 1);  break;
    case 'w': chr_format(cont, "%ld", w->cols); break;
    case 'h': chr_format(cont, "%ld", w->rows); break;
    case 'x': chr_format(cont, "%ld", w->scrx); break;
    case 'y': chr_format(cont, "%ld", w->scry); break;

    case 'b': chr_format(cont, "%ld", buf_len(bf)); break;
    case 'p':
        percent = (100 *(w->scry + w->rows)) / buf_len(bf);
        if (percent > 100) percent = 100;
        chr_format(cont, "%d", percent);
        break;

    case 'n': chr_format(cont, "%s",  buf_get_name(bf));        break;
    case 'm': chr_format(cont, "%s",  bind_info_curr()->name);  break;
    case 'f': chr_format(cont, "%s",  file_name(&(bf->finfo))); break;

    case '%': chr_from_str(cont, "%"); break;
    case 'X': break;

    default: return -1;
    }

    *str += 1;
    return 0;
}

void bar_get_content(bar *b, vec *cont)
{
    char *c, *str;
    size_t colind;
    str = c = b->format;
    col contcol = bar_cont_col;
    col prev    = bar_cont_col;

    colind = 0;

    while (*c)
    {
        if (*c == '%')
        {
            chr_from_mem(cont, str, c - str);
            c += 1;

            if (bar_append_format(b, cont, &c) == -1)
                col_parse(&contcol, &c);

            str = c;
        }
        else
            c += 1;

        if (*c == '\0')
            chr_from_mem(cont, str,  c - str);

        while (colind < vec_len(cont))
        {
            chr *ch;
            ch = vec_get(cont, colind);
            ch->fnt = prev;
            colind += 1;
        }

        prev = contcol;
    }
}

/* Functions for the bar cursor */

void bar_ins(bar *b, vec *chrs)
{
    vec_ins(b->typed, b->ind, vec_len(chrs), vec_first(chrs));
    b->ind += vec_len(chrs);

    bar_out(b);
}

void bar_back(bar *b)
{
    if (b->ind == 0) return;

    b->ind -= 1;

    bar_del(b);
}

void bar_del(bar *b)
{
    vec *typ;
    typ = b->typed;

    if (b->ind >= (int)vec_len(typ))
        return;

    vec_del(typ, b->ind, 1);


    bar_out(b);
}

void bar_move(bar *b, int n)
{
    int len;
    len = vec_len(b->typed);

    b->ind += n;

    if      (b->ind < 0)   b->ind = 0;
    else if (b->ind > len) b->ind = len;

    bar_out(b);
}

void bar_run(bar *b)
{
    vec tmp;
    bar_scrollback_new(b);

    if (b->cb)
    {
        vec_init(&tmp, sizeof(chr));
        vec_cpy(&tmp, b->typed);

        b->cb(b->w, &tmp);

        vec_kill(&tmp);
    }

    bar_cancel(b);
}

void bar_cancel(bar *b)
{
    bar_scrollback_reset(b);

    vec_clr(b->typed);
    vec_clr(&(b->prompt));

    b->ind = 0;
    b->cb  = NULL;

    bar_out(b);
}

void bar_scrollback(bar *b, int dir)
{
    circvec *scr;
    int used;

    scr = &(b->scrollback);

    b->scrind += dir;
    used = circvec_get_used(scr);

    if (b->scrind < 0)     b->scrind = 0;
    if (b->scrind >= used) b->scrind = used - 1;

    b->typed = circvec_get(scr, b->scrind);
}

void bar_scrollback_new(bar *b)
{
    circvec *scr;
    vec *v;

    scr = &(b->scrollback);

    v = circvec_get(scr, -1);

    if (v && v != b->typed)
        vec_cpy(v, b->typed);

    if (v && vec_len(v) == 0)
        return;

    if (circvec_full(scr))
    {
        v = circvec_push(scr);
        vec_clr(v);
    }
    else
    {
        v = circvec_push(scr);
        vec_init(v, sizeof(chr));
    }
}

void bar_scrollback_reset(bar *b)
{
    circvec *scr;
    scr = &(b->scrollback);

    b->scrind = circvec_get_used(scr) - 1;
    b->typed  = circvec_get(scr, b->scrind);
}

/* Create a bar */

void bar_query(bar *b, vec *prompt, void (*cb)(win *w, vec *chrs))
{
    bar_cancel(b);

    vec_cpy(&(b->prompt), prompt);
    b->cb = cb;

    bar_out(b);
}

/* Render the bar */

void bar_out(bar *b)
{
    vec chrs;
    size_t ind, len, typedlen;
    int    needsfree = 1;

    vec_init(&chrs, sizeof(chr));
    win *w;

    w = b->w;

    typedlen = vec_len(b->typed);

    bar_get_content(b, &chrs);
    ind = vec_len(&chrs);

    vec_cpy(&chrs, &(b->prompt));

    len = vec_len(&chrs);
    for (; ind < len; ind++)
        chr_set_cols(vec_get(&chrs, ind), bar_prompt_col);

    vec_cpy(&chrs, b->typed);
    if (typedlen && (int)typedlen != b->ind)
        vec_app(&chrs, &CHR(" "));

    if (vec_len(&(b->prompt)))
        win_add_cur((cur){ .cn = b->ind + ind }, (cur){ .ln = 1 }, 0, &chrs, &needsfree);

    len = vec_len(&chrs);
    for (; ind < len; ind++)
        chr_set_cols(vec_get(&chrs, ind), bar_typed_col);

    if (w->cols > (ssize_t)len)
        vec_rep(&chrs, len, 1, &bar_blank_chr, w->cols - len);

    out_goto(w->xpos + 1, w->ypos + w->rows, stdout);
    out_chrs(vec_first(&chrs), w->cols, 0, stdout);

    vec_kill(&chrs);
}
