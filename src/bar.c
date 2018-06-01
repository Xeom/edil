#include <string.h>

#include "out.h"
#include "bar.h"

#define FMT_ARGS \
    pri.ln + 1, pri.cn + 1, \
    sec.ln + 1, sec.cn + 1, \
    w->cols,    w->rows,    \
    w->scrx,    w->scry,    \
    buf_get_name(buf)

#define ARG_PRI_LN "%1$ld"
#define ARG_PRI_CN "%2$ld"
#define ARG_SEC_LN "%3$ld"
#define ARG_SEC_CN "%4$ld"
#define ARG_COLS   "%5$ld"
#define ARG_ROWS   "%6$ld"
#define ARG_SCRX   "%7$ld"
#define ARG_SCRY   "%8$ld"
#define ARG_NAME   "%9$s"

chr bar_blank_chr = {
    .utf8 = "\xe2\x94\x80", .fnt = { .fg = col_black, .bg = col_black | col_bright }
};

col_desc bar_cont_col = {
    .inv = col_rev, .fg = col_null, .bg = col_null
};

col_desc bar_prompt_col = {
    .inv = col_rev, .fg = col_null, .bg = col_black | col_bright
};

col_desc bar_typed_col = {
    .inv = col_rev, .fg = col_null, .bg = col_null
};

char *bar_format_default = " " ARG_NAME " " ARG_PRI_LN "\xc2\xb7" ARG_PRI_CN " ";

void bar_init(bar *b, win *w)
{
    b->ind = 0;
    b->w   = w;
    b->cb  = NULL;

    vec_init(&(b->typed),  sizeof(chr));
    vec_init(&(b->prompt), sizeof(chr));

    b->format = NULL;
    bar_set_format(b, bar_format_default);
}

void bar_kill(bar *b)
{
    vec_kill(&(b->typed));
    vec_kill(&(b->prompt));

    free(b->format);
}

void bar_set_format(bar *b, char *fmt)
{
    if (b->format)
        free(b->format);

    b->format = malloc(strlen(fmt) + 1);
    strcpy(b->format, fmt);
}

void bar_get_content(bar *b, vec *cont)
{
    cur pri, sec;
    buf *buf;
    win *w;

    w   = b->w;
    buf = w->b;

    pri = w->pri;
    sec = w->sec;

    chr_format(cont, b->format, FMT_ARGS);
}

void bar_ins(bar *b, vec *chrs)
{
    vec_ins(&(b->typed), b->ind, vec_len(chrs), vec_first(chrs));
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
    typ = &(b->typed);

    if (b->ind >= (int)vec_len(typ))
        return;

    vec_del(typ, b->ind, 1);


    bar_out(b);
}

void bar_move(bar *b, int n)
{
    int len;
    len = vec_len(&(b->typed));

    b->ind += n;

    if      (b->ind < 0)   b->ind = 0;
    else if (b->ind > len) b->ind = len;

    bar_out(b);
}

void bar_run(bar *b)
{
    if (b->cb)
        b->cb(b->w, &(b->typed));

    bar_cancel(b);
}

void bar_cancel(bar *b)
{
    vec_clr(&(b->prompt));
    vec_clr(&(b->typed));

    b->ind = 0;
    b->cb  = NULL;

    bar_out(b);
}

void bar_query(bar *b, vec *prompt, void (*cb)(win *w, vec *chrs))
{
    bar_cancel(b);

    vec_cpy(&(b->prompt), prompt);
    b->cb = cb;

    bar_out(b);
}

void bar_out(bar *b)
{
    vec chrs;
    size_t ind, len, typedind;
    int    needsfree = 1;

    vec_init(&chrs, sizeof(chr));
    win *w;

    w = b->w;

    bar_get_content(b, &chrs);

    len = vec_len(&chrs);
    for (ind = 0; ind < len; ind++)
        chr_set_cols(vec_get(&chrs, ind), bar_cont_col);

    vec_cpy(&chrs, &(b->prompt));

    len = vec_len(&chrs);
    for (; ind < len; ind++)
        chr_set_cols(vec_get(&chrs, ind), bar_prompt_col);

    vec_cpy(&chrs, &(b->typed));
    if (vec_len(&(b->typed)) && vec_len(&(b->typed)) != b->ind)
        vec_app(&chrs, &CHR(" "));

    typedind = ind;
    win_add_cur((cur){ .cn = b->ind + typedind }, (cur){ .ln = 1 }, 0, &chrs, &needsfree);

    len = vec_len(&chrs);
    for (; ind < len; ind++)
        chr_set_cols(vec_get(&chrs, ind), bar_typed_col);

    if (w->cols > (ssize_t)len)
        vec_rep(&chrs, len, 1, &bar_blank_chr, w->cols - len);

    out_goto(w->xpos + 1, w->ypos + w->rows, stdout);
    out_chrs(vec_first(&chrs), w->cols, 0, stdout);

    vec_kill(&chrs);
}
