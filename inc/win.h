#if !defined(WIN_H)
# define WIN_H
# include <stdio.h>
# include <unistd.h>
# include "cur.h"

typedef struct win_s win;

struct win_s
{
    /* Dimensions */
    ssize_t scrx, scry;
    ssize_t rows, cols;
    ssize_t xpos, ypos;

    /* Text and cursors */
    buf   *b;
    cur pri, sec;

    /* Stuff for bar mode */
    vec     bartyped;
    vec     barprompt;
    ssize_t barcur;
    void  (*barcb)(win *w, vec *chrs);
};

extern win *win_cur;

void win_init(win *w, buf *b);

vec *win_line(win *w, size_t ln);

ssize_t win_max_ln(win *w);
ssize_t win_max_cn(win *w);
ssize_t win_min_ln(win *w);
ssize_t win_min_cn(win *w);

void win_bar_ins(win *w, vec *chrs);
void win_bar_move(win *w, int n);
void win_bar_back(win *w);
void win_bar_del(win *w);
void win_bar_run(win *w);
void win_bar_query(win *w, vec *prompt, void (*cb)(win *w, vec *chrs));

void win_kill(win *w);

void win_out_bar(win *w, FILE *f);

void win_out_line(win *w, cur c, FILE *f);

void win_out_after(win *w, cur c, FILE *f);

void win_show_cur(win *w, cur c, FILE *f);

#endif
