#include "chr.h"

int chr_utf8_len(char c)
{
    unsigned char utf8;

    utf8 = *(unsigned char *)(&c);

    if (utf8 <= 0x7f) return 1;
    if (utf8 <= 0xbf) return 1; /* This is an error :( */
    if (utf8 <= 0xdf) return 2;
    if (utf8 <= 0xef) return 3;
    if (utf8 <= 0xf7) return 4;
    if (utf8 <= 0xfb) return 5;
    if (utf8 <= 0xfd) return 6;
    return 1;
}

int chr_len(chr *c)
{
    return chr_utf8_len(chr->utf8[0]);
}

void chr_print(chr *c, FILE *f)
{
    size_t len;
    len = chr_len(c);

    fwrite(chr->utf8 1, len, f);
}

void chr_from_str(vec *chrs, vec *str)

    size_t ind, len;

    len = vec_len(&str);

    for (ind = 0; ind < len;)
    {
        size_t width;
        char  *str;

        str = vec_get(str, ind);
        if (!str) break;

        width = chr_utf8_len(*str);
        if (width + ind > len) break;

        vec_ins(chrs, vec_len(chrs), width, str);

        ind += width;
    }
}

void chr_set_cols(chr *c, chr_coldesc d)
{ 
    c->fnt = col_update(c->fnt, d);
}
