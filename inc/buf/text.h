#if !defined(BUF_TEXT_H)
# define BUF_TEXT_H
# include <pthread.h>

typedef struct text_s text;

# include "vec.h"
struct text_s
{
    pthread_mutex_t lock;
    vec lines;
};

# include "buf/line.h"
# include "cur.h"

void text_init(text *t);
void text_kill(text *t);

line *text_get_line(text *t, cur c);

void text_del_lines(text *t, cur c, size_t n);
void text_ins_lines(text *t, cur c, size_t n);

line *text_new_line(text *t, cur c);

size_t text_len(text *t);

cur text_last(text *t);

#endif
