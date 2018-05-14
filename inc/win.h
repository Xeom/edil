#if !defined(WIN_H)
# define WIN_H
# include <stdio.h>
# include <unistd.h>

# if !defined(BAR_H)
 typedef struct win_s win;
#endif

# include "bar.h"
# include "cur.h"

struct win_s
{
    /* Dimensions */
    ssize_t scrx, scry;
    ssize_t rows, cols;
    ssize_t xpos, ypos;

    /* Text and cursors */
    buf   *b;
    cur pri, sec;

    /* Bar at the bottom */
    bar basebar;
};

extern win *win_cur;

void win_init(win *w, buf *b);

vec *win_line(win *w, size_t ln);

ssize_t win_max_ln(win *w);
ssize_t win_max_cn(win *w);
ssize_t win_min_ln(win *w);
ssize_t win_min_cn(win *w);

void win_kill(win *w);

void win_out_line(win *w, cur c);
void win_out_after(win *w, cur c);

void win_buf_out_line(buf *b, cur c);
void win_buf_out_after(buf *b, cur c);

void win_out_all(void);

void win_show_cur(win *w, cur c);

void win_reset(win *w);

void win_set_buf(win *w, buf *b);

#endif
