#include <string.h>
#include <stdarg.h>

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
    return chr_utf8_len(c->utf8[0]);
}

void chr_print(chr *c, FILE *f)
{
    size_t len;
    len = chr_len(c);

    if (chr_is_blank(c))
        return;

    fwrite(c->utf8, 1, len, f);
}

int chr_is_whitespace(chr *c)
{
    if (chr_is_blank(c)            ||
        strcmp(c->utf8, " ")  == 0 ||
        strcmp(c->utf8, "\t") == 0)
        return 1;
    else
        return 0;
}

void chr_from_vec(vec *chrs, vec *str)
{
    size_t len, ind;
    len = vec_len(str);

    ind = chr_from_mem(chrs, vec_get(str, 0), len);

    vec_del(str, 0, ind);
}

size_t chr_from_str(vec *chrs, char *str)
{
    size_t len;

    len = strlen(str);

    return chr_from_mem(chrs, str, len);
}

size_t chr_from_mem(vec *chrs, char *str, size_t len)
{
    size_t ind;

    for (ind = 0; ind < len;)
    {
        size_t width;
        char  *bytes;
        chr    c = CHR("");

        bytes = str + ind;

        width = chr_utf8_len(*bytes);
        if (width + ind > len) break;

        memcpy(c.utf8, bytes, width);

        vec_ins(chrs, vec_len(chrs), 1, &c);

        ind += width;
    }

    return ind;
}

void chr_to_str(vec *chrs, vec *str)
{
    size_t ind, len;
    len = vec_len(chrs);

    for (ind = 0; ind < len; ind++)
    {
        size_t width;

        chr *c;
        c = vec_get(chrs, ind);
        if (!c) break;

        if (chr_is_blank(c)) continue;

        width = chr_len(c);

        vec_ins(str, vec_len(str), width, c->utf8);
    }
}

void chr_set_cols(chr *c, col_desc d)
{
    c->fnt = col_update(c->fnt, d);
}

static char *chr_blank_utf8 = "\377\377\377\377\377\377";

int chr_is_blank(chr *c)
{
    return strncmp(c->utf8, chr_blank_utf8, sizeof(c->utf8)) == 0;
}

void chr_blankify(chr *c)
{
    memcpy(c->utf8, chr_blank_utf8, sizeof(c->utf8));
}

void chr_format(vec *chrs, char *fmt, ...)
{
    va_list args;
    char *buf;
    size_t len, written;
    len = 32;
    buf = NULL;

    do
    {
        va_start(args, fmt);

        len *= 2;
        buf = realloc(buf, len);

        written = vsnprintf(buf, len, fmt, args);
    } while (written >= len);

    va_end(args);

    chr_from_str(chrs, buf);

    free(buf);
}

int chr_scan(vec *chrs, char *fmt, ...)
{
    va_list args;
    vec str;
    int rtn;

    va_start(args, fmt);

    vec_init(&str, sizeof(char));
    chr_to_str(chrs, &str);
    vec_ins(&str, vec_len(&str), 1, NULL);

    rtn = sscanf(vec_get(&str, 0), fmt, args);

    vec_kill(&str);

    return rtn;
}
