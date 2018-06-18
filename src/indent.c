#include <string.h>

#include "print.h"
#include "text/chr.h"
#include "text/col.h"
#include "text/cur.h"
#include "win.h"
#include "ring.h"
#include "text/line.h"

#include "indent.h"

indent_flag indent_mode      = 0;
int         indent_tab_width = 8;
int         indent_lvl_width = 8;
char       *indent_tab_text  = ";";
col_desc    indent_tab_col   = { .fg = col_blue, .bg = col_none };

void indent_print_tab(size_t ind, FILE *f, col fnt)
{
    char *str;
    size_t len, width;
    col tabcol;

    width = indent_get_width(&CHR("\t"), ind);
    len   = strlen(indent_tab_text);

    tabcol = col_update(fnt, indent_tab_col);
    col_print(tabcol, f);

    if (width > len)
    {
        size_t n;
        n = width - len;

        while (n--) print_str(" ");

        str = indent_tab_text;
    }
    else
    {
        str = indent_tab_text + len - width;
    }

    print_str(str);

    col_print(fnt, f);
}

int indent_get_width(chr *c, size_t ind)
{
    if (!c) return 0;

    if (chr_is_blank(c))
        return 1;

    if (strcmp(c->utf8, "\t") == 0)
        return indent_tab_width - (ind % indent_tab_width);

    return 1;
}

void indent_add_blanks_buf(buf *b)
{
    ssize_t len;
    cur loc;

    len = buf_len(b);
    for (loc = (cur){0, 0}; loc.ln < len; ++(loc.ln))
    {
        line *l;
        l = buf_get_line(b, loc);

        indent_add_blanks_line(line_vec(l), 0);

        line_unlock(l);
    }
}

void indent_add_blanks_line(vec *line, size_t ind)
{
    for (; ind < vec_len(line); ind++)
    {
        chr *c;
        c = vec_get(line, ind);

        if (!c) break;

        if (indent_get_width(c, ind) != 1 || strcmp(c->utf8, "\t") == 0)
            indent_add_blanks_chr(line, ind);
    }
}

void indent_add_blanks_chr(vec *line, size_t ind)
{
    size_t blanks, width, blankind, len;
    chr   *c;

    c   = vec_get(line, ind);
    len = vec_len(line);

    if (!c) return;

    width = indent_get_width(c, ind);

    for (blankind = ind + 1; blankind < len; blankind++)
    {
        c = vec_get(line, blankind);

        if (!c || !chr_is_blank(c)) break;
    }

    blanks = blankind - ind - 1;

    if (blanks > width - 1)
    {
        size_t n;
        n = blanks - width + 1;
        vec_del(line, blankind - n, n);
    }
    else if (blanks < width - 1)
    {
        size_t n, off;
        n = width - 1 - blanks;
        vec_ins(line, blankind, n, NULL);

        for (off = 0; off < n; off++)
        {
            chr *c;
            c = vec_get(line, blankind + off);
            chr_blankify(c);
        }
    }
}

size_t indent_get_depth(buf *b, cur c)
{
    line *l;
    size_t depth, len;

    l = buf_get_line(b, c);
    if (!l) return 0;

    len = line_len(l);
    for (depth = 0; depth < len; ++depth)
    {
        chr *c;
        c = line_chr(l, (cur){ .cn = depth });
        if (!chr_is_whitespace(c)) break;
    }

    line_unlock(l);
    return depth;
}

int indent_is_blank(buf *b, cur c)
{
    line *l;
    size_t ind;

    l = buf_get_line(b, c);
    if (!l) return 1;

    ind = line_len(l);
    while (ind--)
    {
        chr *c;
        c = line_chr(l, (cur){ .cn = ind });
        if (!chr_is_whitespace(c))
        {
            line_unlock(l);
            return 0;
        }
    }

    line_unlock(l);
    return 1;
}

void indent_set_depth(buf *b, cur c, size_t depth)
{
    line *l;
    size_t orig;

    c.cn = 0;

    if (b->flags & buf_readonly) return;
    b->flags |= buf_modified;

    orig = indent_get_depth(b, c);

    l = buf_get_line(b, c);
    if (!l) return;

    if (orig) line_del(l, c, orig);

    if (depth)
    {
        vec whitespace;
        size_t tabs, spaces;
        chr tab   = CHR("\t");
        chr space = CHR(" ");

        if (indent_mode & indent_spacify)
        {
            spaces = depth;
            tabs   = 0;
        }
        else
        {
            spaces = depth % indent_tab_width;
            tabs   = depth / indent_tab_width;
        }

        vec_init(&whitespace, sizeof(chr));

        vec_rep(&whitespace, 0, 1, &space, spaces);
        vec_rep(&whitespace, 0, 1, &tab,   tabs);

        line_ins(l, c, &whitespace);

        vec_kill(&whitespace);
    }

    line_unlock(l);
}

void indent_trim_end(buf *b, cur c)
{
    size_t n;
    line  *l;

    if (b->flags & buf_readonly) return;
    b->flags |= buf_modified;

    if (!(indent_mode & indent_trim)) return;

    l = buf_get_line(b, c);
    if (!l) return;

    c.cn = line_len(l);
    while (c.cn--)
    {
        chr *cr;
        cr = line_chr(l, c);
        if (!cr || !chr_is_whitespace(cr)) break;
    }

    c.cn += 1;
    n = line_len(l) - c.cn;

    line_unlock(l);

    if (n) buf_del(b, c, n);
}

cur indent_incr_depth(buf *b, cur c)
{
    ssize_t depth, orig;
    depth = indent_get_depth(b, c);
    orig  = depth;

    depth += indent_lvl_width;
    depth -= depth % indent_lvl_width;

    indent_set_depth(b, c, depth);

    if (c.cn <= orig)
        c.cn = depth;
    else
        c.cn += depth - orig;

    cur_chk_bounds(&c, b);
    cur_chk_blank(&c, b, (cur){ .cn = 1 });

    return c;
}

cur indent_decr_depth(buf *b, cur c)
{
    ssize_t depth, orig;
    depth = indent_get_depth(b, c);
    orig  = depth;

    depth -= indent_lvl_width;
    if (depth < 0) depth = 0;

    if (depth % indent_lvl_width)
        depth += indent_lvl_width - (depth % indent_lvl_width);

    indent_set_depth(b, c, depth);

    if (c.cn <= orig)
        c.cn = depth;
    else
        c.cn += depth - orig;

    if (c.cn < 0) c.cn = 0;

    cur_chk_bounds(&c, b);
    cur_chk_blank(&c, b, (cur){ .cn = -1 });

    return c;
}

cur indent_auto_depth(buf *b, cur c)
{
    ssize_t depth;
    cur loc;

    if (!(indent_mode & indent_auto))
        return c;

    loc.ln = c.ln - 1;
    loc.cn = 0;

    if (indent_mode & indent_skipblank)
    {
        while (loc.ln > 0 && indent_is_blank(b, loc))
            loc.ln -= 1;
    }

    depth = indent_get_depth(b, loc);
    if (depth < 0) depth = 0;

    indent_set_depth(b, c, depth);
    c.cn = depth;

    return c;
}

void indent_set_tab_width(size_t width)
{
    size_t ind, len;
    indent_tab_width = width;

    len = vec_len(&ring_bufs);
    for (ind = 0; ind < len; ind++)
    {
        buf **b;
        b = vec_get(&ring_bufs, ind);
        indent_add_blanks_buf(*b);
    }

    win_out_all();
}
