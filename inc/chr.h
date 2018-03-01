#if !defined(CHR_H)
# define CHR_H
# include "col.h"
# include "vec.h"

typedef struct chr_s chr;

struct chr_s
{
    char utf8[6];
    col  fnt;
};

int chr_utf8_len(char c);

int chr_len(chr *c);

void chr_print(chr *c, FILE *f);

void chr_from_vec(vec *chrs, vec *str);

size_t chr_from_str(vec *chrs, char *str, size_t len);

void chr_set_cols(chr *c, col_desc d);

int chr_is_blank(chr *c);

void chr_blankify(chr *c);

#endif
