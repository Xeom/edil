#if !defined(CHR_H)
# define CHR_H
# include "types.h"

#define CHR(_utf8) (chr){                                \
    .utf8 = _utf8,                                       \
    .fnt = { .fg = col_none, .bg = col_none, .attr = 0 } \
}

/* Get the length of a utf8 character, if starting with c */
int chr_utf8_len(char c);

/* Get the utf8 width of a chr */
int chr_len(chr *c);

int chr_is_whitespace(chr *c);

/* Print a chr in utf8 format, with no colours, to a file */
void chr_print(chr *c, FILE *f);

/* Turn a utf8 chars string into chrs. The vec chrs is appended with *
 * the chr representation of the chars in str. These are assumed to  *
 * be appropriately initialized.                                     */
void chr_from_vec(vec *chrs, vec *str);
size_t chr_from_str(vec *chrs, char *str);
size_t chr_from_mem(vec *chrs, char *str, size_t len);

/* Compare a vector of chrs to a string, returns the same values as *
 * an equivialent strcmp.                                            */
int chr_cmp_str(vec *chrs, char *str);

/* Turn a vec of chrs back into utf8 chars. */
void chr_to_str(vec *chrs, vec *str);

void chr_set_cols(chr *c, col_desc d);

int chr_is_blank(chr *c);

void chr_blankify(chr *c);

/* Append a formatted string to an array of chrs */
void chr_format(vec *chrs, char *fmt, ...);

/* Perform sscanf on a vec of chrs */
int chr_scan(vec *chrs, char *fmt, ...);

#endif
