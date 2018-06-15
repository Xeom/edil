#if !defined(BUF_H)
# define BUF_H
# include "types.h"

void buf_init(buf *b);

void buf_kill(buf *b);

void buf_setcol(buf *b, cur loc, size_t n, col_desc col);

void buf_ins(buf *b, cur loc, chr *chrs, size_t n);

void buf_del(buf *b, cur loc, size_t n);

void buf_clr(buf *b);

void buf_ins_nl(buf *b, cur loc);

void buf_del_nl(buf *b, cur loc);

void buf_ins_line(buf *b, cur loc);

void buf_del_line(buf *b, cur loc);

size_t buf_len(buf *b);

ssize_t buf_line_len(buf *b, cur loc);

vec *buf_line(buf *b, cur loc);

chr *buf_chr(buf *b, cur loc);

cur buf_last_cur(buf *b);

void buf_ins_buf(buf *b, cur *c, buf *other, cur loc, cur end);

char *buf_get_name(buf *b);

void buf_set_name(buf *b, char *name);

#endif
