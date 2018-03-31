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

cur cur_enter(cur c, buf *b);

cur cur_check_bounds(cur c, buf *b);
cur cur_check_blank(cur c, buf *b, cur dir);

cur cur_move(cur c, buf *b, cur dir);

cur cur_home(cur c, buf *b);
cur cur_end(cur c,  buf *b);
cur cur_pgup(cur c, win *w);
cur cur_pgdn(cur c, win *w);

cur cur_del(cur c, buf *b);

cur cur_ins(cur c, buf *b, vec *text);

#endif
