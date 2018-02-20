#if !defined(OUT_H)
# define OUT_H
# include <stdio.h>
# include "col.h"

extern col out_blank_line_col;
extern char *out_blank_line_text;
extern col_desc out_cur1_col_desc;
extern col_desc out_cur2_col_desc;

void out_goto(int cn, int ln, FILE *f);

void out_blank_line(FILE *f);

void out_chrs(vec *chrs, size_t len, FILE *f);

void out_line(buf *b, size_t len, cur c, FILE *f)

void out_lines_after(but *b, cur c, FILE *f);

void out_init(FILE *f);

void out_kill(FILE *f);

#endif
