col col_update(col c, col_desc d)
{
    c.attr &= ~(d.del);
    c.attr |=   d.set;
    c.attr ^=   d.inv;

    if (d.fg != col_null) c.fg = d.fg;
    if (d.bg != col_null) c.bg = d.bg;

    return f;
}

void col_print(col c, FILE *f)
{
    fputs("\033[0", f);

    if (col.fg < col_none)
    {
        if (col.fg & col_bright) fputs(";1", f);
        fprintf(f, ";%d", (col & col_allcols) + 30);
    }

    if (col.attrs & col_under) fputs(";4", f);
    if (col.attrs & col_rev)   fputs(";7", f);
    if (col.attrs & col_blink) fputs(";5", f);

    if (col.bg < col_none)
    {
        if (col.bg & col_bright) 
            fprintf(f, ";%d", (col & col_allcols) + 100);
        else
            fprintf(f, ";%d", (col & col_allcols) + 40);
    }
}
