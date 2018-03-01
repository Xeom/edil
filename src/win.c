#include <string.h>

#include "out.h"

#include "win.h"

static int  win_out_goto(win *w, cur *c, FILE *f);
static vec *win_add_cur(win *w, cur c, vec *line, int *needsfree);

win *win_cur;

chr      win_pri_chr = { .utf8 = "\xc2\xab", .fnt = { .fg = col_none, .bg = col_black | col_bright } };
col_desc win_pri_col = { .inv = col_rev,   .fg = col_null, .bg = col_null };

chr      win_sec_chr = { .utf8 = "\xc2\xab", .fnt = { .fg = col_none, .bg = col_none } };
col_desc win_sec_col = { .inv = col_under, .fg = col_null, .bg = col_null };

void win_init(win *w, buf *b)
{
    memset(w, 0, sizeof(win));
    w->rows = 10;
    w->cols = 10;
    w->b    = b;
    vec_init(&(w->bartyped),  sizeof(chr));
    vec_init(&(w->barprompt), sizeof(chr));
}

void win_kill(win *w)
{
    vec_kill(&(w->bartyped));
    vec_kill(&(w->barprompt));
}

vec *win_line(win *w, size_t ln)
{
    return vec_get(&(w->b->lines), ln);
}

ssize_t win_max_ln(win *w)
{
    return w->scry + w->rows - 2;
}

ssize_t win_max_cn(win *w)
{
    return w->scrx + w->cols - 1;
}

ssize_t win_min_ln(win *w)
{
    return w->scry;
}

ssize_t win_min_cn(win *w)
{
    return w->scrx;
}

static int win_out_goto(win *w, cur *c, FILE *f)
{
    if (c->ln < win_min_ln(w) 
     || c->ln > win_max_ln(w))
        return 0;

    if (c->cn > win_max_cn(w))
        return 0;

    if (c->cn < win_min_cn(w))
        c->cn = win_min_cn(w);

    out_goto(c->cn - w->scrx + w->xpos + 1, c->ln - w->scry + w->ypos + 1, f);

    return 1;
}

void win_bar_fill(win *w, vec *bar)
{
    char  *fstr = "%ld,%ld ";
    char   curstr[32];
    vec    curvec;
    size_t len;

    len = snprintf(curstr, sizeof(curstr), fstr, w->pri.ln + 1, w->pri.cn + 1);

    vec_init(&curvec, sizeof(char));
    vec_ins(&curvec, 0, len, curstr);

    chr_from_str(bar, &curvec);

    vec_ins(bar, vec_len(bar), vec_len(&(w->barprompt)), vec_get(&(w->barprompt), 0));
    vec_ins(bar, vec_len(bar), vec_len(&(w->bartyped)),  vec_get(&(w->bartyped),  0));

    vec_kill(&curvec);
}

void win_out_bar(win *w, FILE *f)
{   
    vec    bar;
    size_t outlen;

    vec_init(&bar, sizeof(chr));
    win_bar_fill(w, &bar);

    outlen = vec_len(&bar);
    if (w->cols < (ssize_t)outlen)
        outlen = w->cols;

    out_goto(1 - w->scrx + w->xpos, w->rows - w->scry + w->ypos, f);
    out_chrs(vec_get(&bar, 0), outlen, f);

    vec_kill(&bar);
}

static vec *win_add_cur(win *w, cur c, vec *line, int *needsfree)
{
    size_t linelen;

    linelen = vec_len(line);

    *needsfree = 0;

    if (w->pri.ln == c.ln || w->sec.ln == c.ln)
    {
        vec *modline;

        modline = malloc(sizeof(vec));
        vec_init(modline, sizeof(chr));
        vec_ins(modline, 0, linelen, vec_get(line, 0));

        line = modline;
        *needsfree = 1;
    }
  
    if (w->pri.ln == c.ln)
    {
        chr *curchr;

        if (w->pri.cn == (ssize_t)linelen)
            vec_ins(line, linelen++, 1, &win_pri_chr);

        curchr = vec_get(line, w->pri.cn);
        if (curchr)
            chr_set_cols(curchr, win_pri_col);
    }

    if (w->sec.ln == c.ln && !(w->pri.cn == w->sec.cn && w->pri.ln == w->sec.ln))
    {
        chr *curchr;

        if (w->sec.cn == (ssize_t)linelen)
            vec_ins(line, linelen++, 1, &win_sec_chr);

        curchr = vec_get(line, w->sec.cn);
        if (curchr) 
            chr_set_cols(curchr, win_sec_col);
    }

    return line;
}

void win_out_line(win *w, cur c, FILE *f)
{
    int      needsfree;
    vec     *line;
    size_t   outlen;

    if (win_out_goto(w, &c, f) == 0) return;

    if (c.ln >= (ssize_t)buf_len(w->b))
    {
        out_blank_line(f);
        return;
    }

    line = win_line(w, c.ln);
    if (!line) return;

    line = win_add_cur(w, c, line, &needsfree);

    outlen = vec_len(line) - c.cn - w->scrx;
    if ((ssize_t)outlen > w->cols) outlen = w->cols;

    out_chrs(vec_get(line, c.cn + w->scrx), outlen, f);

    if (needsfree) vec_kill(line);

}

void win_out_after(win *w, cur c, FILE *f)
{
    if (c.ln < win_min_ln(w))
        c = (cur){ .ln = w->scry, .cn = 0 };

    while (c.ln <= win_max_ln(w))
    {
        win_out_line(w, c, f);
        c = (cur){ .ln = c.ln + 1, .cn = 0 };
    }
}

