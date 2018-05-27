#include <string.h>
#include <limits.h>

#include "indent.h"

#include "cur.h"

#define CHK_FLAGS(b) \
    { if ((b)->flags & buf_readonly) return; \
      (b)->flags    |= buf_modified; }

#define PRI_SEC \
    (cur *[]){ &(w->pri), &(w->sec) }, 2

static void cur_set_rel_pos(cur c, buf *b, cur *affect[], int numaffect, cur rel[]);
static void cur_get_rel_pos(cur c, buf *b, cur *affect[], int numaffect, cur rel[]);

static int cur_can_shift_line(win *w, ssize_t ln, int dir);

int cur_chk_bounds(cur *c, buf *b)
{
    ssize_t len;
    cur     prev;

    prev = *c;

    len = buf_len(b);
    if      (c->ln <  0)   c->ln = 0;
    else if (c->ln >= len) c->ln = len - 1;

    len = buf_line_len(b, *c);
    if      (c->cn <  0)   c->cn = 0;
    else if (c->cn >= len) c->cn = len;

    return (memcpy(&prev, c, sizeof(cur)) == 0) ? 0 : -1;
}

int cur_chk_blank(cur *c, buf *b, cur dir)
{
    ssize_t len;
    cur     prev;

    prev = *c;

    len = buf_line_len(b, *c);
    while (c->cn > 0 && c->cn < len)
    {
        if (!chr_is_blank(buf_chr(b, *c)))
            break;

        if (dir.ln != 0 || dir.cn < 0)
            c->cn -= 1;
        else
            c->cn += 1;
    }

    return (memcpy(&prev, c, sizeof(cur)) == 0) ? 0 : -1;
}

static void cur_get_rel_pos(cur c, buf *b, cur *affect[], int numaffect, cur rel[])
{
    int ind;
    for (ind = 0; ind < numaffect; ++ind)
    {
        cur a;
        a = *affect[ind];

        rel[ind].ln = a.ln - c.ln;
        rel[ind].cn = a.cn - c.cn;
    }
}

static void cur_set_rel_pos(cur c, buf *b, cur *affect[], int numaffect, cur rel[])
{
    int ind;
    for (ind = 0; ind < numaffect; ++ind)
    {
        cur r, *a, prev, delta;
        a = affect[ind];
        r = rel[ind];
        prev = *a;

        /* If we're on the same line */
        if (r.ln == 0)
        {
            /* c has not changed lines but moved from after to before a */
            if (a->ln == c.ln && r.cn < 0 && a->cn > c.cn) a->cn = c.cn;
            /* after original c */
            if (r.cn >= 0)
            {
                a->cn = c.cn + r.cn;
                a->ln = c.ln;
            }
        }

        /* if we're on a line after c */
        if (r.ln >= 0) a->ln = c.ln + r.ln;

        /* if we're on a line before c and c has moved before a */
        if (r.ln < 0 && c.ln < a->ln) a->ln = MIN(c.ln, a->ln);

        delta = *a;
        delta.ln -= prev.ln;
        delta.cn -= prev.cn;

        cur_chk_bounds(a, b);
        cur_chk_blank(a, b, delta);
    }
}

void cur_del(cur c, buf *b, cur *affect[], int numaffect)
{
    cur rel[numaffect];
    ssize_t len;

    len = buf_line_len(b, c);
    if (len == c.cn)
    {
        cur pretendprev = { .ln = c.ln + 1 };

        cur_get_rel_pos(pretendprev, b, affect, numaffect, rel);
        buf_del_nl(b, c);
        cur_set_rel_pos(c, b, affect, numaffect, rel);
    }
    else
    {
        cur pretendprev = c;
        pretendprev.cn += indent_get_width(buf_chr(b, c), c.ln);;

        cur_get_rel_pos(pretendprev, b, affect, numaffect, rel);
        buf_del(b, c, 1);
        cur_set_rel_pos(c, b, affect, numaffect, rel);
    }
}

void cur_ins(cur c, buf *b, vec *text, cur *affect[], int numaffect)
{
    cur rel[numaffect];
    ssize_t origlen, newlen;

    cur_get_rel_pos(c, b, affect, numaffect, rel);
    origlen = buf_line_len(b, c);
    buf_ins(b, c, vec_first(text), vec_len(text));
    newlen  = buf_line_len(b, c);

    c.cn += newlen - origlen;
    cur_set_rel_pos(c, b, affect, numaffect, rel);
}

void cur_enter(cur c, buf *b, cur *affect[], int numaffect)
{
    cur rel[numaffect];

    cur_get_rel_pos(c, b, affect, numaffect, rel);
    buf_ins_nl(b, c);
    c.cn  = 0;
    c.ln += 1;
    cur_set_rel_pos(c, b, affect, numaffect, rel);
}

/* * * * * * * * * * *
 * Window  functions *
 * * * * * * * * * * */

void cur_ins_win(win *w, vec *text)
{
    cur  prev;
    buf *b;
    b = w->b;
    CHK_FLAGS(b);

    prev = w->pri;
    cur_ins(w->pri, b, text, PRI_SEC);

    win_out_line(w, prev);
}

void cur_del_win(win *w)
{
    int isnl;
    buf *b;
    b = w->b;
    CHK_FLAGS(b);

    isnl = (w->pri.cn == buf_line_len(b, w->pri));

    cur_del(w->pri, b, PRI_SEC);

    if (isnl)
        win_out_after(w, w->pri);
    else
        win_out_line(w, w->pri);

}

void cur_enter_win(win *w)
{
    buf *b;
    cur prev;
    b = w->b;
    CHK_FLAGS(b);

    prev = w->pri;
    cur_enter(w->pri, b, PRI_SEC);

    win_out_after(w, prev);
}

void cur_move_win(win *w, cur dir)
{
    ssize_t len;
    cur *c, prev;

    c = &(w->pri);
    prev = *c;

    len = buf_line_len(w->b, w->pri);

    c->cn += dir.cn;
    c->ln += dir.ln;

    if (c->cn > len)        *c = (cur){ .ln = c->ln + 1 };
    if (c->cn < 0 && c->ln) *c = (cur){ .ln = c->ln - 1, .cn = LONG_MAX };
    cur_chk_bounds(c, w->b);
    cur_chk_blank(c,  w->b, (cur){ .ln = -1 });


    if (c->ln == prev.ln)
    {
        win_out_line(w, CUR_START(w->pri, prev));
    }
    else
    {
        win_out_line(w, w->pri);
        win_out_line(w, prev);
    }
}

void cur_home_win(win *w)
{
    cur *c;
    c = &(w->pri);

    c->cn = 0;
    win_out_line(w, *c);
}

void cur_end_win(win *w)
{
    cur *c;
    ssize_t len;
    c   = &(w->pri);
    len = buf_line_len(w->b, *c);

    c->cn = len;
    win_out_line(w, *c);
}

void cur_pgup_win(win *w)
{
    cur *c;
    c = &(w->pri);

    c->ln = win_min_ln(w) - 1;

    cur_chk_bounds(c, w->b);
    cur_chk_blank(c,  w->b, (cur){ .ln = -1 });

    win_out_after(w, (cur){0, 0});
}

void cur_pgdn_win(win *w)
{
    cur *c;
    c = &(w->pri);

    c->ln = win_max_ln(w) + 1;

    cur_chk_bounds(c, w->b);
    cur_chk_blank(c,  w->b, (cur){ .ln = 1 });

    win_out_after(w, (cur){0, 0});
}

void cur_lineify_win(win *w)
{
    cur prevsec;
    ssize_t len;

    prevsec = w->sec;

    len = buf_line_len(w->b, w->pri);
    w->pri.cn = len;

    w->sec.cn = 0;
    w->sec.ln = w->pri.ln;

    win_out_line(w, w->sec);

    if (prevsec.ln != w->sec.ln)
        win_out_line(w, CUR_START(prevsec, w->pri));
}

static int cur_can_shift_line(win *w, ssize_t ln, int dir)
{
    ssize_t cn1, cn2, len;

    cn1 = MIN(w->pri.cn, w->sec.cn);
    cn2 = MAX(w->pri.cn, w->sec.cn) + 1;

    len = buf_line_len(w->b, (cur){ .ln = ln });

    if (cn1 > len) return 1;
    if (cn2 < len) return 1;

    return 0;
}

static void cur_shift_line(win *w, ssize_t ln, int dir)
{
    ssize_t cn1, cn2, len;
    cur cur1, cur2;
    chr tmp, *c;

    cn1 = MIN(w->pri.cn, w->sec.cn);
    cn2 = MAX(w->pri.cn, w->sec.cn) + 1;

    cur1 = (cur){ .ln = ln, .cn = cn1 };
    cur2 = (cur){ .ln = ln, .cn = cn2 };

    len = buf_line_len(w->b, cur1);

    if (cn1 > len) return;

    if (dir == 1)
    {
        c = buf_chr(w->b, cur2);
        tmp = *c;
        buf_del(w->b, cur2, 1);
        buf_ins(w->b, cur1, &tmp, 1);
    }
    else if (dir == -1)
    {
        c = buf_chr(w->b, cur1);
        tmp = *c;
        buf_ins(w->b, cur2, &tmp, 1);
        buf_del(w->b, cur1, 1);
    }
}

void cur_shift(win *w, cur dir)
{
    cur *start, *end;

    start = CUR_START(&(w->pri), &(w->sec));
    end   = CUR_END  (&(w->pri), &(w->sec));

    if (dir.cn)
    {
        ssize_t ln;

        for (ln = start->ln; ln < end->ln; ++ln)
            if (!cur_can_shift_line(w, ln, dir.cn))
                return;

        for (ln = start->ln; ln < end->ln; ++ln)
            cur_shift_line(w, ln, dir.cn);
    }
    else if (dir.ln > 0)
    {
        vec *line;

        buf_ins_line(w->b, *start);

        line = buf_line(w->b, (cur){ .ln = end->ln + 1 });
        if (!line) return;

        buf_ins(w->b, (cur){ .ln = start->ln }, vec_first(line), vec_len(line));
        buf_del_line(w->b, (cur){ .ln = end->ln + 1 });
    }
    else if (dir.ln < 0)
    {
        vec *line;

        buf_ins_line(w->b, *end);

        line = buf_line(w->b, *start);
        if (!line) return;

        buf_ins(w->b, (cur){ .ln = end->ln + 1 }, vec_first(line), vec_len(line));
        buf_del_line(w->b, *start);
    }

    start->ln += dir.ln;
    end->ln   += dir.ln;
    start->cn += dir.cn;
    end->cn   += dir.cn;

    win_out_after(w, (cur){ .ln = start->ln - 1 });
}

void cur_del_region(win *w)
{
    buf *b;
    cur *start, *end, c;

    b = w->b;

    start = CUR_START(&(w->pri), &(w->sec));
    end   = CUR_END  (&(w->pri), &(w->sec));

    if (start->ln == end->ln)
    {
        buf_del(b, *start, end->cn - start->ln + 1);
    }
    else
    {
        buf_del(b, *start, buf_line_len(b, *start) - start->cn);
        buf_del(b, (cur){ .ln = end->ln }, end->cn + 1);

        for (c.ln = end->ln - 1; c.ln > start->ln; --(c.ln))
            buf_del_line(b, c);

        buf_del_nl(b, *start);
    }

    *end = *start;
    win_out_after(w, *start);
}

void cur_ins_buf(win *w, buf *oth)
{
    buf *b;
    ssize_t len;
    cur from = (cur){0, 0};

    b   = w->b;
    len = buf_len(oth);

    for (; from.ln < len; ++(from.ln))
    {
        vec *text;

        text = buf_line(oth, from);

        if (from.ln != 0)
            cur_enter(w->pri, b, PRI_SEC);

        cur_ins(w->pri, b, text, PRI_SEC);
    }
}