#include "out.h"

#include "win.h"

void win_init(win *w, buf *b)
{
    w->scrx = 0;
    w->scry = 0;
    w->rows = 10;
    w->cols = 10;
    w->xpos = 0;
    w->ypos = 0;
    w->b    = b;
    w->pri  = (cur){0, 0};
    w->sec  = (cur){0, 0};
}

vec *win_line(win *w, size_t ln)
{
    return vec_get(&(w->b->lines), ln);
}

ssize_t win_max_ln(win *w)
{
    return w->scry + w->rows - 1;
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

int win_out_goto(win *w, cur *c, FILE *f)
{
    if (c->ln < win_min_ln(w) 
     || c->ln > win_max_ln(w))
        return 0;

    if (c->cn > win_max_cn(w))
        return 0;

    if (c->cn < win_min_cn(w))
        c->cn = win_min_cn(w);

    out_goto(c->cn - w->scrx + w->xpos, c->ln - w->scry + w->ypos, f);

    return 1;
}

void win_out_line(win *w, cur c, FILE *f)
{
    chr space = { .utf8 = " ", .fnt = { .fg = col_none, .bg = col_none } };
    vec     *line, modline;
    size_t   linelen;

    if (win_out_goto(w, &c, f) == 0) return;

    if (c.ln >= (ssize_t)buf_len(w->b))
    {
        out_blank_line(f);
        return;
    }

    line = win_line(w, c.ln);
    if (!line) return;

    if (w->pri.ln == c.ln || w->sec.ln == c.ln)
    {
        linelen = vec_len(line);

        vec_init(&modline, sizeof(chr));
        vec_ins(&modline, 0, linelen, vec_get(line, 0));

        line = &modline;
    }
  
    if (w->pri.ln == c.ln)
    {
        chr *curchr;

        if (w->pri.cn == (ssize_t)linelen) vec_ins(line, linelen, 1, &space);

        curchr = vec_get(line, w->pri.cn);
        if (curchr) chr_set_cols(curchr, out_cur1_col_desc);
    }

    if (w->sec.ln == c.ln && !(w->pri.cn == w->sec.cn && w->pri.ln == w->sec.ln))
    {
        chr *curchr;

        if (w->sec.cn == (ssize_t)linelen) vec_ins(line, linelen, 1, &space);

        curchr = vec_get(line, w->sec.cn);
        if (curchr) chr_set_cols(curchr, out_cur2_col_desc);
    }

    out_chrs(vec_get(line, c.cn + w->scrx), linelen - c.cn - w->scrx, win_max_cn(w) - c.cn, f);

    if (line == &modline) vec_kill(line);
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


