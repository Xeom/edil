#include "col.h"

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
    case 2: c->bg   = bg;
    case 1: c->fg   = fg;
    }

    *str += len[matched - 1];

    return 0;
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
    fputs("\033[0", f);
#if !defined(COL_NONE)
    if (c.fg < col_none)
    {
        if (c.fg & col_bright) fputs(";1", f);
        fprintf(f, ";%d", (c.fg & col_allcols) + 30);
    }
#endif
    if (c.attr & col_under) fputs(";4", f);
    if (c.attr & col_rev)   fputs(";7", f);
    if (c.attr & col_blink) fputs(";5", f);

#if !defined(COL_NONE)
    if (c.bg < col_none)
    {
        if (c.bg & col_bright)
            fprintf(f, ";%d", (c.bg & col_allcols) + 100);
        else
            fprintf(f, ";%d", (c.bg & col_allcols) + 40);
    }
#endif

    fputs("m", f);
}

