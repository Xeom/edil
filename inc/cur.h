#if !defined(CUR_H)
# define CUR_H
# include <unistd.h>
# include "types.h"

# if !defined(MAX)
#  define MAX(a, b) (((a) > (b)) ? (a) : (b))
# endif

# if !defined(MIN)
#  define MIN(a, b) (((a) > (b)) ? (b) : (a))
# endif

static inline ssize_t cur_cn_ptr(cur *c) { return c->cn; }
static inline ssize_t cur_ln_ptr(cur *c) { return c->ln; }
static inline ssize_t cur_cn(cur c)      { return c.cn; }
static inline ssize_t cur_ln(cur c)      { return c.ln; }

# define _CUR_CN(c) _Generic((c), cur: cur_cn, cur *: cur_cn_ptr)(c)
# define _CUR_LN(c) _Generic((c), cur: cur_ln, cur *: cur_ln_ptr)(c)

# define CUR_GREATER(a, b) \
    (_CUR_LN(a)  > _CUR_LN(b) || \
    (_CUR_LN(a) == _CUR_LN(b) && _CUR_CN(a) > _CUR_CN(b)))

# define CUR_START(a, b) ((CUR_GREATER(a, b)) ? (b) : (a))
# define CUR_END(a, b)   ((CUR_GREATER(a, b)) ? (a) : (b))

void cur_set_rel_pos(cur c, buf *b, cur *affect[], int numaffect, cur rel[]);
void cur_get_rel_pos(cur c, buf *b, cur *affect[], int numaffect, cur rel[]);

int cur_chk_bounds(cur *c, buf *b);
int cur_chk_blank(cur *c, buf *b, cur dir);

void cur_del(cur c, buf *b, cur *affect[], int numaffect);
void cur_ins(cur c, buf *b, vec *text, cur *affect[], int numaffect);
void cur_enter(cur c, buf *b, cur *affect[], int numaffect);

void cur_ins_win(win *w, vec *text);
void cur_del_win(win *w);
void cur_enter_win(win *w);
void cur_enter_line_win(win *w);

void cur_move_win(win *w, cur dir);
void cur_home_win(win *w);
void cur_end_win(win *w);
void cur_pgup_win(win *w);
void cur_pgdn_win(win *w);
void cur_lineify_win(win *w);

void cur_shift(win *w, cur dir);

void cur_del_region(win *w);
void cur_ins_buf(win *w, buf *oth);

void cur_ins_long_win(win *w, vec *text);
void cur_del_long_win(win *w);

#endif
