#if !defined(BUF_H)
# define BUF_H
# include "types.h"
# include "text/text.h"

static inline line *buf_get_line (buf *b, cur c) { return text_get_line(&(b->t), c); }
static inline line *buf_new_line (buf *b, cur c) { return text_new_line(&(b->t), c); }

static inline void buf_del_lines(buf *b, cur c, size_t n) { text_del_lines(&(b->t), c, n); }
static inline void buf_ins_lines(buf *b, cur c, size_t n) { text_ins_lines(&(b->t), c, n); }

static inline size_t buf_len(buf *b)  { return text_len (&(b->t)); }
static inline cur    buf_last(buf *b) { return text_last(&(b->t)); }

void buf_init(buf *b);
void buf_kill(buf *b);

void buf_ins(buf *b, cur loc, vec *chrs);
void buf_ins_str(buf *b, cur loc, char *str);
void buf_ins_mem(buf *b, cur loc, size_t n, chr *mem);

void buf_del(buf *b, cur loc, size_t n);

void buf_clr(buf *b);

ssize_t buf_line_len(buf *b, cur loc);

void buf_ins_nl(buf *b, cur loc);
void buf_del_nl(buf *b, cur loc);

chr buf_chr(buf *b, cur loc);

void buf_ins_from(buf *b, cur c, buf *oth, cur loc, cur end);

void buf_set_name(buf *b, char *name);
char *buf_get_name(buf *b);

#endif
