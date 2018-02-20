#if !defined(CUR_H)
# define CUR_H

typedef enum
{
    cmd_mode_bar;
    cmd_mode_buf;
    cmd_mode_mnu;
    cmd_mode_sel;
    cmd_mode_rct;   
} cur_mode_type;

typedef struct cur_s cur;
struct cur_s
{
    ssize_t cn, ln;
};

extern cur_mode_type cur_mode;

cur cur_enter(cur c, buf *b);

cur cur_check_bounds(cur c, buf *b);

cur cur_move(cur c, buf *b, cur dir);

cur cur_del(cur c, buf *b);

cur cur_ins(cur c, buf *b, vec *text);

#endif
