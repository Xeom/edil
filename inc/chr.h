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

/* Get the length of a utf8 character, if starting with c */
int chr_utf8_len(char c);

/* Get the utf8 width of a chr */
int chr_len(chr *c);

/* Print a chr in utf8 format, with no colours, to a file */
void chr_print(chr *c, FILE *f);

/* Turn a utf8 chars string into chrs. The vec chrs is appended with *
 * the chr representation of the chars in str. These are assumed to  *
 * be appropriately initialized.                                     */
void chr_from_vec(vec *chrs, vec *str);
size_t chr_from_str(vec *chrs, char *str, size_t len);

/* Turn a vec of chrs back into utf8 chars. */
void chr_to_str(vec *chrs, vec *str);

void chr_set_cols(chr *c, col_desc d);

int chr_is_blank(chr *c);

void chr_blankify(chr *c);

/* Append a formatted string to an array of chrs */
void chr_format(vec *chrs, char *fmt, ...);

#endif
