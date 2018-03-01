#if !defined(WIN_H)
# define WIN_H
# include <stdio.h>
# include <unistd.h>
# include "cur.h"

typedef struct win_s win;

struct win_s
{
    ssize_t scrx, scry;
    ssize_t rows, cols;
    ssize_t xpos, ypos;
    vec    bartyped;
    vec    barprompt;
    size_t barcur;
    buf   *b;
    cur pri, sec;
};

extern win *win_cur;

void win_init(win *w, buf *b);

vec *win_line(win *w, size_t ln);

ssize_t win_max_ln(win *w);
ssize_t win_max_cn(win *w);
ssize_t win_min_ln(win *w);
ssize_t win_min_cn(win *w);

void win_kill(win *w);

void win_bar_fill(win *w, vec *bar);

void win_out_bar(win *w, FILE *f);

void win_out_line(win *w, cur c, FILE *f);

void win_out_after(win *w, cur c, FILE *f);

#endif
