#include "indent.h"

#include "cur.h"

cur_mode_type cur_mode;

cur cur_check_bounds(cur c, buf *b)
{
    size_t len;

    len = buf_len(b);

    if (c.ln <  0) c.ln = 0;
    else if (c.ln >= (ssize_t)len) c.ln = len - 1;

    len = buf_line_len(b, c);

    if (c.cn < 0) c.cn = 0;
    else if (c.cn >  (ssize_t)len) c.cn = len;

    return c;
}

/* Make sure a cursor isn't inside a multi-column character */
cur cur_check_blank(cur c, buf *b, cur dir)
{
    vec *line;
    size_t len;

    line = vec_get(&(b->lines), c.ln);

    if (!line) return c;

    len  = vec_len(line);

    while (c.cn != 0 && c.cn != (ssize_t)len)
    {
        if (!chr_is_blank(vec_get(line, c.cn)))
            break;

        if (dir.ln != 0 || dir.cn < 0)
            c.cn -= 1;
        else
            c.cn += 1;
    }

    return c;
}

void cur_move(win *w, cur dir)
{
    cur c, prev;
    c = w->pri;

    c.ln += dir.ln;
    c.cn += dir.cn;

    if (c.cn < 0)
    {
        c.ln -= 1;
        c.cn  = buf_line_len(w->b, c);
    }

    if (c.cn > (ssize_t)buf_line_len(w->b, c) &&
        c.ln < (ssize_t)buf_len(w->b) - 1)
    {
        if (dir.ln == 0)
        {
            c.ln += 1;
            c.cn  = 0;
        }
        else
        {
            c.cn = buf_line_len(w->b, c);
        }
    }

    c = cur_check_bounds(c, w->b);
    c = cur_check_blank(c, w->b, dir);

    prev = w->pri;
    w->pri = c;

    if (c.ln == prev.ln)
    {
        win_out_line(w, (prev.cn < c.cn) ? prev : c);
    }
    else
    {
        win_out_line(w, prev);
        win_out_line(w, c);
    }
}

void cur_home(win *w)
{
    w->pri.cn = 0;
    win_out_line(w, w->pri);
}

void cur_end(win *w)
{
    w->pri.cn = (ssize_t)buf_line_len(w->b, w->pri);
    win_out_line(w, w->pri);
}

void cur_pgdn(win *w)
{
    cur c;
    c = w->pri;

    c.ln = win_max_ln(w) + 1;

    c = cur_check_bounds(c, w->b);
    c = cur_check_blank(c, w->b, (cur){ .ln = 1 });

    w->pri = c;

    win_out_after(w, (cur){0, 0});
}

void cur_pgup(win *w)
{
    cur c;
    c = w->pri;

    c.ln = win_min_ln(w) - 1;

    c = cur_check_bounds(c, w->b);
    c = cur_check_blank(c, w->b, (cur){ .ln = -1 });

    w->pri = c;

    win_out_after(w, (cur){0, 0});
}

void cur_del(win *w)
{
    cur c;
    size_t len;

    c = w->pri;

    if (w->b->flags & buf_readonly) return;
    w->b->flags |= buf_modified;

    len = buf_line_len(w->b, c);

    if ((ssize_t)len == c.cn)
    {
        vec *line;
        size_t num;
        cur delcur = { .ln = c.ln + 1, .cn = 0 };

        line = vec_get(&(w->b->lines), c.ln + 1);
        if (!line) return;

        num = vec_len(line);

        buf_ins(w->b, c, vec_get(line, 0), num);
        buf_del_line(w->b, delcur);

        win_out_after(w, w->pri);
    }
    else
    {
        buf_del(w->b, w->pri, 1);
        win_out_line(w, w->pri);
    }
}

void cur_ins(win *w, vec *text)
{
    size_t num;
    cur    c, prev;

    c = w->pri;

    if (w->b->flags & buf_readonly) return;
    w->b->flags |= buf_modified;

    num = vec_len(text);

    buf_ins(w->b, c, vec_get(text, 0), num);
    c.cn += num;

    c = cur_check_bounds(c, w->b);
    c = cur_check_blank(c, w->b, (cur){ .cn = 1 });

    prev = w->pri;
    w->pri = c;
    win_out_after(w, prev);
}

void cur_enter(win *w)
{
    cur    c, prev, rtn;
    size_t len;

    c = w->pri;

    len = buf_line_len(w->b, c);
    rtn = (cur){ .ln = c.ln + 1, .cn = 0 };

    if (w->b->flags & buf_readonly) return;
    w->b->flags |= buf_modified;

    buf_ins_line(w->b, rtn);

    if ((ssize_t)len > c.cn)
    {
        vec *line;
        size_t num;

        line = vec_get(&(w->b->lines), c.ln);
        if (!line) return;

        num = len - c.cn;

        buf_ins(w->b, rtn, vec_get(line, c.cn), num);
	    buf_del(w->b, c,   num);
    }

    rtn = indent_auto_depth(w->b, rtn);

    prev = w->pri;
    w->pri = rtn;
    win_out_after(w, prev);
}

