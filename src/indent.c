#include <string.h>

#include "ring.h"

#include "indent.h"

indent_flag indent_mode      = 0;
int         indent_tab_width = 4;
int         indent_lvl_width = 4;
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

        while (n--) fputs(" ", f);

        str = indent_tab_text;
    }
    else
    {
        str = indent_tab_text + len - width;
    }

    fputs(str, f);
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
    vec *lines;
    size_t ln, maxln;

    lines = &(b->lines);
    maxln = vec_len(lines);
    for (ln = 0; ln < maxln; ln++)
    {
        vec *l;
        l = vec_get(lines, ln);

        indent_add_blanks_line(l, 0);
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
    vec *line;
    size_t ind, len;

    line = vec_get(&(b->lines), c.ln);
    if (!line) return 0;

    len = vec_len(line);
    for (ind = 0; ind < len; ind++)
    {
        chr *c;
        c = vec_get(line, ind);
        if (!c || !chr_is_whitespace(c)) break;
    }

    return ind;
}

int indent_is_blank(buf *b, cur c)
{
    vec *line;
    size_t ind;

    line = vec_get(&(b->lines), c.ln);
    if (!line) return 0;

    ind = vec_len(line);
    while (ind--)
    {
        chr *c;
        c = vec_get(line, ind);
        if (!c || !chr_is_whitespace(c))
            return 0;
    }

    return 1;
}

void indent_set_depth(buf *b, cur c, size_t depth)
{
    vec *line;
    size_t orig;

    c.cn = 0;

    if (b->flags & buf_readonly) return;
    b->flags |= buf_modified;

    line = vec_get(&(b->lines), c.ln);
    if (!line) return;

    orig = indent_get_depth(b, c);

    if (orig)
        buf_del(b, c, orig);

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

        buf_ins(b, c, vec_first(&whitespace), vec_len(&whitespace));

        vec_kill(&whitespace);
    }

    indent_add_blanks_line(line, 0);
}

void indent_trim_end(buf *b, cur c)
{
    vec *line;
    size_t len;

    if (b->flags & buf_readonly) return;
    b->flags |= buf_modified;

    if (!(indent_mode & indent_trim)) return;

    line = vec_get(&(b->lines), c.ln);
    if (!line) return;

    len = vec_len(line);
    c.cn = len;
    while (c.cn--)
    {
        chr *cr;
        cr = vec_get(line, c.cn);
        if (!cr || !chr_is_whitespace(cr)) break;
    }

    c.cn += 1;

    buf_del(b, c, len - c.cn);
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

    c = cur_check_bounds(c, b);
    c = cur_check_blank(c, b, (cur){ .cn = 1 });

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

    c = cur_check_bounds(c, b);
    c = cur_check_blank(c, b, (cur){ .cn = -1 });

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

