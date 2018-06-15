#if !defined(BUF_TEXT_H)
# define BUF_TEXT_H
# include "types.h"

void text_init(text *t);
void text_kill(text *t);

void text_cpy_line(text *t, cur c, vec *v);

/* YOU CAN ONLY GET OR NEW ONE LINE AT ONCE !!! */
line *text_get_line(text *t, cur c);

void text_del_lines(text *t, cur c, size_t n);
void text_ins_lines(text *t, cur c, size_t n);

line *text_new_line(text *t, cur c);

ssize_t text_len(text *t);

cur text_last(text *t);

#endif
