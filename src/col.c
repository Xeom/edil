#include "col.h"

col col_default = { .fg = col_none, .bg = col_none, .attr = 0 };

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

