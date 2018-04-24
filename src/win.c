#include <string.h>

#include "out.h"
#include "ui.h"
#include "file.h"

#include "win.h"

static int  win_out_goto(win *w, cur *c);
static vec *win_add_cur(cur pri, cur sec, ssize_t ln, vec *line, int *tofree);

static void win_bar_fill_fname(win *w, vec *bar);
static void win_bar_fill_query(win *w, vec *bar);
static void win_bar_fill_pos(win *w, vec *bar);

static void win_bar_fill(win *w, vec *bar);

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
    .inv = col_under, .fg = col_blue, .bg = col_null
};

void win_init(win *w, buf *b)
{
    memset(w, 0, sizeof(win));
    w->rows = 10;
    w->cols = 10;
    w->b    = b;
    w->pri  = (cur){0, 0};
    w->sec  = (cur){0, 0};
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

void win_bar_fill_pos(win *w, vec *bar)
{
    chr_format(
        bar,
        " %ld %ld\xc2\xb7%ld",
        w->scry, w->pri.ln + 1, w->pri.cn + 1
    );
}

void win_bar_fill_fname(win *w, vec *bar)
{
    file *f;
    f = &(w->b->finfo);

    if (file_associated(f))
        chr_format(bar, " (%s)", vec_first(&(f->basename)));
}

static void win_bar_fill_query(win *w, vec *bar)
{
    int tofree;
    vec *prompt, *typed;
    cur barcur;

    tofree = 0;

    typed  = &(w->bartyped);
    prompt = &(w->barprompt);
    barcur = (cur){ .cn = w->barcur };

    /* Format the bar cursor, {.ln = 1} just ensures no second cur is shown */
    if (ui_mode == ui_mode_bar)
        typed = win_add_cur(barcur, (cur){ .ln = 1 }, 0, typed, &tofree);

    if (vec_len(prompt))
        chr_format(bar, " ");

    vec_cpy(bar, prompt);
    vec_cpy(bar, typed);

    if (tofree)
    {
        vec_kill(typed);
        free(typed);
    }
}

static void win_bar_fill(win *w, vec *bar)
{
    win_bar_fill_pos(w, bar);
    win_bar_fill_fname(w, bar);
    win_bar_fill_query(w, bar);

    chr_format(bar, " ");
}

void win_out_bar(win *w)
{
    vec    bar;
    size_t len, ind;

    vec_init(&bar, sizeof(chr));
    win_bar_fill(w, &bar);

    len = vec_len(&bar);

    if (w->cols > (ssize_t)len)
    {
        vec_ins(&bar, len, w->cols - len, NULL);

        for (ind = len; (ssize_t)ind < w->cols; ind++)
            memcpy(vec_get(&bar, ind), &win_bar_chr, sizeof(chr));
    }

    for (ind = 0; (ssize_t)ind < w->cols; ind++)
    {
        chr *c;
        c = vec_get(&bar, ind);
        chr_set_cols(c, win_bar_col);
    }

    out_goto(w->xpos + 1, w->ypos + w->rows, stdout);
    out_chrs(vec_first(&bar), w->cols, 0, stdout);

    vec_kill(&bar);
}

void win_bar_ins(win *w, vec *chrs)
{
    vec *v;

    v = &(w->bartyped);
    vec_ins(v, w->barcur, vec_len(chrs), vec_first(chrs));
    w->barcur += vec_len(chrs);
}

void win_bar_back(win *w)
{
    if (w->barcur == 0) return;

    w->barcur -= 1;
    win_bar_del(w);
}

void win_bar_del(win *w)
{
    vec *v;

    v = &(w->bartyped);
    if (w->barcur >= (ssize_t)vec_len(v)) return;

    vec_del(v, w->barcur, 1);
}

void win_bar_move(win *w, int n)
{
    size_t len;

    len = vec_len(&(w->bartyped));
    w->barcur += n;
    if (w->barcur < 0)   w->barcur = 0;
    if (w->barcur > (ssize_t)len) w->barcur = len;
}

void win_bar_run(win *w)
{
    if (w->barcb) w->barcb(w, &(w->bartyped));

    win_bar_cancel(w);
}

void win_bar_query(win *w, vec *prompt, void (*cb)(win *w, vec *chrs))
{
    win_bar_cancel(w);

    vec_cpy(&(w->barprompt), prompt);
    w->barcb = cb;
}

void win_bar_cancel(win *w)
{
    vec_clr(&(w->barprompt));
    vec_clr(&(w->bartyped));

    w->barcur = 0;
    w->barcb = NULL;
}

static vec *win_add_cur(cur pri, cur sec, ssize_t ln, vec *line, int *mod)
{
    size_t linelen;

    linelen = vec_len(line);

    *mod = 0;

    if (pri.ln == ln || sec.ln == ln)
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
    int      needsfree;
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

    line = win_add_cur(w->pri, w->sec, c.ln, line, &needsfree);

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
