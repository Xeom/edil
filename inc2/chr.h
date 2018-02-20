#if !defined(CHR_H)
# define CHR_H
typedef struct chr_s chr;

struct chr_s
{
    char utf8[6];
    col  fnt;
};

int chr_utf8_len(char c);

int chr_len(chr *c);

void chr_print(chr *c, FILE *f);

void chr_from_str(vec *chrs, vec *str);

void chr_set_cols(chr *c, chr_coldesc c);

#endif
