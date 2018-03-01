#include <string.h>

#include "out.h"

#include "win.h"

static int  win_out_goto(win *w, cur *c, FILE *f);
static vec *win_add_cur(cur pri, cur sec, ssize_t ln, vec *line, int *needsfree);

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
    int needsfree;
    char  *fstr = "%ld,%ld ";
    char   curstr[32];
    vec   *typed, *prompt;
    size_t len;

    len = snprintf(curstr, sizeof(curstr), fstr, w->pri.ln + 1, w->pri.cn + 1, vec_len(&(w->bartyped)));

    chr_from_str(bar, curstr, len);

    typed  = win_add_cur((cur){ .cn = w->barcur }, (cur){ .ln = 1 }, 0, &(w->bartyped), &needsfree);
    prompt = &(w->barprompt); 

    vec_ins(bar, vec_len(bar), vec_len(prompt), vec_get(prompt, 0));
    vec_ins(bar, vec_len(bar), vec_len(typed),  vec_get(typed,  0));

    if (needsfree)
    {
        vec_kill(typed);
        free(typed);
    }
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

void win_bar_ins(win *w, vec *chrs)
{
    vec *v;

    v = &(w->bartyped);
    vec_ins(v, w->barcur, vec_len(chrs), vec_get(chrs, 0));
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
    if (w->barcb) w->barcb(w, &(w->barprompt));
    w->barcb = NULL;

    vec_del(&(w->barprompt), 0, vec_len(&(w->barprompt)));
    vec_del(&(w->bartyped),  0, vec_len(&(w->bartyped)));

    w->barcur = 0;
}

void win_bar_query(win *w, vec *prompt, void (*cb)(win *w, vec *chrs))
{
    if (w->barcb) (w->barcb(w, NULL));

    vec_del(&(w->barprompt), 0, vec_len(&(w->barprompt)));
    vec_ins(&(w->barprompt), 0, vec_len(prompt), vec_get(prompt, 0));

    vec_del(&(w->bartyped), 0, vec_len(&(w->bartyped)));
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
        vec_ins(modline, 0, linelen, vec_get(line, 0));

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

    line = win_add_cur(w->pri, w->sec, c.ln, line, &needsfree);

    outlen = vec_len(line) - c.cn - w->scrx;
    if ((ssize_t)outlen > w->cols) outlen = w->cols;

    out_chrs(vec_get(line, c.cn + w->scrx), outlen, f);

    if (needsfree) 
    {
        vec_kill(line);
        free(line);
    }
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

