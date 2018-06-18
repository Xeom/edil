#include "text/chr.h"
#include "print.h"

#include "container/vec.h"

#include "text/col.h"

col col_default = { .fg = col_none, .bg = col_none, .attr = 0 };

int col_parse(col *c, char **str)
{
    int matched, len[3];
    unsigned int fg, bg;
    unsigned int attrs;

    matched = sscanf(
        *str,
        "%u%n,%u%n,%u%n",
        &fg, &len[0], &bg, &len[1], &attrs, &len[2]
    );

    switch (matched)
    {
    case 0: return -1;
    case 3: c->attr = attrs;
        /* Falls through */
    case 2: c->bg   = bg;
        /* Falls through */
    case 1: c->fg   = fg;
    }

    *str += len[matched - 1];

    return 0;
}

void col_parse_string(col c, vec *chrs, char *str)
{
    char *seg;
    size_t colind;
    col prev;

    prev   = c;
    colind = vec_len(chrs);
    seg    = str;

    while (*str)
    {
        if (*str == '%')
        {
            chr_from_mem(chrs, seg, str++ - seg);
            col_parse(&c, &(str));
            seg = str;
        }
        else
            ++str;

        if (*(str) == '\0')
        {
            chr_from_mem(chrs, seg, str - seg);
        }

        while (colind < vec_len(chrs))
        {
            chr *ch;
            ch = vec_get(chrs, colind);
            ch->fnt = prev;
            colind += 1;
        }

        prev = c;
    }
}

col col_update(col c, col_desc d)
{
    c.attr &= ~(d.del);
    c.attr |=   d.set;
    c.attr ^=   d.inv;

    if (d.fg != col_null) c.fg = d.fg;
    if (d.bg != col_null) c.bg = d.bg;

    return c;
}

void col_print(col c, FILE *f)
{
    print_str("\033[0");

#if !defined(COL_NONE)
    if (c.fg < col_none)
    {
        if (c.fg & col_bright) print_str(";1");
        print_fmt(";%d", (c.fg &col_allcols) + 30);
    }
#endif
    if (c.attr & col_under) print_str(";4");
    if (c.attr & col_rev)   print_str(";7");
    if (c.attr & col_blink) print_str(";5");

#if !defined(COL_NONE)
    if (c.bg < col_none)
    {
        if (c.bg & col_bright)
            print_fmt(";%d", (c.bg & col_allcols) + 100);
        else
            print_fmt(";%d", (c.bg & col_allcols) + 40);
    }
#endif

    print_str("m");
}
