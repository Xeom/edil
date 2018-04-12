#if !defined(CUR_H)
# define CUR_H
# include "vec.h"

# include <unistd.h>

typedef enum
{
    cmd_mode_bar,
    cmd_mode_buf,
    cmd_mode_mnu,
    cmd_mode_sel,
    cmd_mode_rct
} cur_mode_type;

typedef struct cur_s cur;

struct cur_s
{
    ssize_t cn, ln;
};

extern cur_mode_type cur_mode;

/* Down here because these require cur */
# include "buf.h"
# include "win.h"

void cur_enter(win *w);

cur cur_check_bounds(cur c, buf *b);
cur cur_check_blank(cur c, buf *b, cur dir);

void cur_move(win *w, cur dir);

void cur_home(win *w);
void cur_end(win *w);
void cur_pgup(win *w);
void cur_pgdn(win *w);

void cur_del(win *w);
void cur_ins(win *w, vec *text);

int cur_greater(cur a, cur b);

cur *cur_region_start(win *w);
cur *cur_region_end(win *w);

void cur_move_region(win *w, cur dir);
#endif
