#if !defined(OUT_H)
# define OUT_H
# include <stdio.h>
# include "types.h"

extern col out_blank_line_col;
extern char *out_blank_line_text;
extern col_desc out_cur1_col_desc;
extern col_desc out_cur2_col_desc;
extern int out_to_resize;

extern ssize_t out_rows, out_cols;

void out_log(vec *chrs, FILE *f);

void out_goto(int cn, int ln, FILE *f);

void out_clr_line(FILE *f);

void out_blank_line(FILE *f);

void out_chrs(chr *chrs, size_t n, size_t off, FILE *f);

void out_init(FILE *f);

void out_kill(FILE *f);

#endif
