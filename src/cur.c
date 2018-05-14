#include <string.h>

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

void cur_lineify(win *w)
{
    cur prev;
    prev = w->sec;

    w->pri.cn = (ssize_t)buf_line_len(w->b, w->pri);
    w->sec = (cur){ .ln = w->pri.ln };

    win_out_line(w, w->sec);

    if (prev.ln != w->pri.ln)
        win_out_line(w, prev);
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
        buf_del_nl(w->b, c);

        if (w->sec.ln > c.ln)      w->sec.ln -= 1;
        if (w->sec.ln == c.ln + 1) w->sec.cn += len;

        win_out_after(w, w->pri);
    }
    else
    {
        buf_del(w->b, w->pri, 1);

        if (w->sec.ln == c.ln && w->sec.cn > c.ln)
            w->sec.cn -= 1;

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

    buf_ins(w->b, c, vec_first(text), num);
    c.cn += num;

    c = cur_check_bounds(c, w->b);
    c = cur_check_blank(c, w->b, (cur){ .cn = 1 });

    prev = w->pri;
    w->pri = c;

    if (w->sec.ln == prev.ln && w->sec.cn >= prev.cn)
        w->sec.cn += 1;

    win_out_after(w, prev);
}

void cur_enter(win *w)
{
    cur prev, rtn;

    rtn = (cur){ .ln = w->pri.ln + 1 };

    if (w->b->flags & buf_readonly) return;
    w->b->flags |= buf_modified;

    buf_ins_nl(w->b, w->pri);

    rtn = indent_auto_depth(w->b, rtn);

    indent_trim_end(w->b, w->pri);

    prev = w->pri;
    w->pri = rtn;

    if (w->sec.ln > prev.ln)
    {
        w->sec.ln += 1;
    }
    else if (memcmp(&(w->sec), &prev, sizeof(cur)) == 0)
    {
        w->sec.ln += 1;
        w->sec.cn  = 0;
    }

    win_out_after(w, (cur){ .ln = prev.ln });
}

int cur_greater(cur a, cur b)
{
    if (a.ln == b.ln)
        return a.cn > b.cn;
    else
        return a.ln > b.ln;
}

cur *cur_region_start(win *w)
{
    if (cur_greater(w->pri, w->sec))
        return &(w->sec);
    else
        return &(w->pri);
}

cur *cur_region_end(win *w)
{
    if (cur_greater(w->pri, w->sec))
        return &(w->pri);
    else
        return &(w->sec);
}

void cur_ins_buf(win *w, buf *other, cur loc, cur end)
{
    cur prev;

    if (w->b->flags & buf_readonly) return;
    w->b->flags |= buf_modified;

    prev = w->pri;

    buf_ins_buf(w->b, &(w->pri), other, loc, end);

    win_out_after(w, prev);
}

int cur_move_region_lr_chk(win *w, ssize_t ln, ssize_t inscn, ssize_t delcn)
{
    ssize_t len;
    len = buf_line_len(w->b, (cur){ .ln = ln });

    if (inscn >= len && delcn >= len) return 1;
    if (inscn < 0 || inscn >= len)  return 0;
    if (delcn < 0 || delcn >= len) return 0;

    return 1;
}

int cur_move_region_lr(win *w, cur dir, ssize_t cn1, ssize_t cn2, ssize_t ln1, ssize_t ln2)
{
    ssize_t inscn, delcn, ln;

    if (dir.cn > 0)
    {
        delcn = cn2 + 1;
        inscn = cn1;
    }
    else
    {
        delcn = cn1 - 1;
        inscn = cn2;
    }

    for (ln = ln1; ln <= ln2; ++ln)
    {
        if (!cur_move_region_lr_chk(w, ln, inscn, delcn))
            return 0;
    }

    for (ln = ln1; ln <= ln2; ++ln)
    {
        cur delcur = { .ln = ln, .cn = delcn };
        cur inscur = { .ln = ln, .cn = inscn };
        chr tmp, *c;

        c = buf_chr(w->b, delcur);

        if (!c) continue;

        tmp = *c;

        buf_del(w->b, delcur, 1);
        buf_ins(w->b, inscur, &tmp, 1);
    }

    return 1;
}

void cur_move_region(win *w, cur dir)
{
    vec tmp;
    cur *start, *end;

    if (w->b->flags & buf_readonly) return;
    w->b->flags |= buf_modified;

    start = cur_region_start(w);
    end   = cur_region_end(w);

    if ((dir.ln < 0 && start->ln > 0) || dir.ln > 0)
    {
        cur delcur, inscur;
        vec *line;

        if (dir.ln > 0) /* Shifting forward */
        {
            /* Delete the line one past the end */
            delcur = (cur){ .ln = end->ln + 1 };
            /* Insert it again at the start */
            inscur = (cur){ .ln = start->ln   };

            start->ln += 1;
            end->ln   += 1;
        }
        else
        {
            /* Delete the line one before the start */
            delcur = (cur){ .ln = start->ln - 1 };
            /* Insert it at the end this time */
            inscur = (cur){ .ln = end->ln       };

            start->ln -= 1;
            end->ln   -= 1;
        }

        vec_init(&tmp, sizeof(chr));

        line = buf_line(w->b, delcur);
        vec_cpy(&tmp, line);

        buf_del_line(w->b, delcur);
        buf_ins_line(w->b, inscur);

        buf_ins(w->b, inscur, vec_first(&tmp), vec_len(&tmp));

        win_out_after(w, (cur){ .ln = start->ln - 1 });

        vec_kill(&tmp);
    }

    if (dir.cn != 0)
    {
        ssize_t cn1, cn2, ln1, ln2;
        int rtn;

        ln1 = start->ln;
        ln2 = end->ln;

        cn1 = (start->cn > end->cn) ? end->cn   : start->cn;
        cn2 = (start->cn > end->cn) ? start->cn : end->cn;

        rtn = cur_move_region_lr(w, dir, cn1, cn2, ln1, ln2);

        if (rtn)
        {
            end->cn   += dir.cn;
            start->cn += dir.cn;
        }

        win_out_after(w, (cur){ .ln = start->ln });
    }
}

void cur_del_region(win *w)
{
    cur *start, *end;
    buf *b;

    b = w->b;

    if (w->b->flags & buf_readonly) return;
    w->b->flags |= buf_modified;

    start = cur_region_start(w);
    end   = cur_region_end(w);

    if (end->ln != start->ln)
    {
        ssize_t numdel, nummov;
        numdel = buf_line_len(b, *start) - start->cn;
        nummov = buf_line_len(b, *end)   - end->cn - 1;

        if (numdel) buf_del(b, *start, numdel);

        if (nummov > 0)
        {
            chr *data;

            data = buf_chr(b, *end);
            buf_ins(b, *start, data + 1, nummov);
        }
    }
    else if (end->ln == start->ln)
    {
        size_t numdel;
        numdel = end->cn - start->cn;

        if (end->cn < buf_line_len(b, *start))
            numdel += 1;

        buf_del(b, *start, numdel);
    }

    while (end->ln > start->ln)
    {
        buf_del_line(b, *end);
        end->ln -= 1;
    }

    *end = *start;
}

void cur_ins_region(win *w, vec *text)
{
}