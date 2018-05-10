#include <string.h>

#include "out.h"
#include "ui.h"
#include "file.h"

#include "win.h"

static int  win_out_goto(win *w, cur *c);
static vec *win_add_cur(cur pri, cur sec, ssize_t ln, vec *line, int *tofree);

win *win_cur = NULL;

chr win_bar_chr = {
    .utf8 = "-", .fnt = { .fg = col_black | col_bright, .bg = col_none }
};
col_desc win_bar_col = {
    .inv = col_rev, .fg = col_null, .bg = col_null
};

chr win_pri_chr = {
    .utf8 = "\xc2\xab", .fnt = { .fg = col_none, .bg = col_blue }
};
col_desc win_pri_col = {
    .inv = col_rev, .fg = col_null, .bg = col_null
};

chr win_sec_chr = {
    .utf8 = "\xc2\xab", .fnt = { .fg = col_none, .bg = col_none }
};
col_desc win_sec_col = {
    .set = col_under, .fg = col_blue, .bg = col_null
};

chr win_trailing_chr = {
    .utf8 = ";", .fnt = { .fg = col_red, .bg = col_none, .attr = 0 }
};

void win_init(win *w, buf *b)
{
    memset(w, 0, sizeof(win));

    w->rows = 10;
    w->cols = 10;
    w->b    = b;
    w->pri  = (cur){0, 0};
    w->sec  = (cur){0, 0};

    bar_init(&(w->basebar), w);
}

void win_kill(win *w)
{
    bar_kill(&(w->basebar));
}

vec *win_line(win *w, size_t ln)
{
    return buf_line(w->b, (cur){ .ln = ln });
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

static int win_out_goto(win *w, cur *c)
{
    if (c->ln < win_min_ln(w)
     || c->ln > win_max_ln(w))
        return 0;

    if (c->cn > win_max_cn(w))
        return 0;

    if (c->cn < win_min_cn(w))
        c->cn = win_min_cn(w);

    out_goto(
        c->cn - w->scrx + w->xpos + 1,
        c->ln - w->scry + w->ypos + 1,
        stdout
    );

    return 1;
}

static vec *win_add_trailing_space(cur pri, cur sec, ssize_t ln, vec *line, int *mod)
{
    chr *c;

    c = vec_get(line, vec_len(line) - 1);

    if (!c) return line;

    if (*(c->utf8) == ' ' && (*mod == 0))
    {
        vec *modline;

        modline = malloc(sizeof(vec));
        vec_init(modline, sizeof(chr));
        vec_cpy(modline,  line);

        *mod = 1;
        c = vec_get(modline, vec_len(modline) - 1);
        *c = win_trailing_chr;

        return modline;
    }

    return line;
}

static vec *win_add_cur(cur pri, cur sec, ssize_t ln, vec *line, int *mod)
{
    size_t linelen;

    linelen = vec_len(line);

    *mod = 0;

    if ((pri.ln == ln || sec.ln == ln) && (*mod == 0))
    {
        vec *modline;

        modline = malloc(sizeof(vec));
        vec_init(modline, sizeof(chr));
        vec_cpy(modline, line);

        line = modline;
        *mod = 1;
    }

    if (pri.ln == ln)
    {
        chr *curchr;

        if (pri.cn == (ssize_t)linelen)
            vec_ins(line, linelen++, 1, &win_pri_chr);

        curchr = vec_get(line, pri.cn);
        if (curchr)
            chr_set_cols(curchr, win_pri_col);
    }

    if (sec.ln == ln && !(pri.cn == sec.cn && pri.ln == sec.ln))
    {
        chr *curchr;

        if (sec.cn == (ssize_t)linelen)
            vec_ins(line, linelen++, 1, &win_sec_chr);

        curchr = vec_get(line, sec.cn);
        if (curchr)
            chr_set_cols(curchr, win_sec_col);
    }

    return line;
}

void win_out_line(win *w, cur c)
{
    int      needsfree = 0;
    vec     *line;
    ssize_t   outlen;

    if (win_out_goto(w, &c) == 0) return;

    if (c.ln >= (ssize_t)buf_len(w->b))
    {
        out_blank_line(stdout);
        return;
    }

    line = win_line(w, c.ln);
    if (!line) return;

    line = win_add_trailing_space(w->pri, w->sec, c.ln, line, &needsfree);
    line = win_add_cur(w->pri, w->sec, c.ln, line, &needsfree);

    if (needsfree)
    {
        c.cn = 0;
        win_out_goto(w, &c);
    }

    outlen = vec_len(line) - c.cn;
    if (outlen < 0)       outlen = 0;
    if (outlen > w->cols) outlen = w->cols;

    out_chrs(vec_get(line, c.cn), outlen, c.cn, stdout);

    if (outlen != w->cols) out_clr_line(stdout);

    if (needsfree)
    {
        vec_kill(line);
        free(line);
    }
}

void win_out_after(win *w, cur c)
{
    if (c.ln < win_min_ln(w))
        c = (cur){ .ln = w->scry, .cn = 0 };

    while (c.ln <= win_max_ln(w))
    {
        win_out_line(w, c);
        c = (cur){ .ln = c.ln + 1, .cn = 0 };
    }
}

void win_show_cur(win *w, cur c)
{
    int needsupdate = 0;

    if (c.cn < win_min_cn(w) || c.cn > win_max_cn(w))
    {
        w->scrx = c.cn - w->cols / 2;
        if (w->scrx < 0) w->scrx = 0;
        needsupdate = 1;
    }

    if (c.ln < win_min_ln(w) || c.ln > win_max_ln(w))
    {
        w->scry = c.ln - w->rows / 2;
        if (w->scry < 0) w->scry = 0;
        needsupdate = 1;
    }

    if (needsupdate)
        win_out_after(w, (cur){0, 0});
}

void win_buf_out_line(buf *b, cur c)
{
    if (b == win_cur->b)
        win_out_line(win_cur, c);
}

void win_buf_out_after(buf *b, cur c)
{
    if (b == win_cur->b)
        win_out_after(win_cur, c);
}

void win_out_all(void)
{
    win_out_after(win_cur, (cur){0, 0});
}

void win_reset(win *w)
{
    w->scrx = 0;
    w->scry = 0;

    w->sec = (cur){0, 0};
    w->pri = cur_check_bounds(w->b->prihint, w->b);

    win_show_cur(w, w->pri);

    win_out_all();
}