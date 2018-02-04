#if !defined(TEXT_H)
# define TEXT_H

# include "vec.h"

typedef enum
{
    text_cmd_setfg,
    text_cmd_ins,
    text_cmd_del,
    text_cmd_ins_line,
    text_cmd_del_line,
} text_cmd_type;

typedef enum
{
    text_col_black   = 0x00,
    text_col_red     = 0x01,
    text_col_green   = 0x02,
    text_col_yellow  = 0x03,
    text_col_blue    = 0x04,
    text_col_magenta = 0x05,
    text_col_cyan    = 0x06,
    text_col_white   = 0x07,
    text_col_none    = 0x08,
    text_col_allcols = 0x0f,
    text_col_bold    = 0x10,
    text_col_under   = 0x20,
    text_col_rev     = 0x40,
    text_col_blink   = 0x80,
    text_col_allflgs = 0xf0
} text_col;

typedef enum
{
    text_flag_vis = 0x1,
    text_flag_lns = 0x2,
    text_flag_ro  = 0x4
} text_flag;

typedef struct text_cmd_s  text_cmd;
typedef struct text_char_s text_char;
typedef struct text_buf_s  text_buf;
typedef struct text_cur_s  text_cur;

struct text_cmd_s
{
    text_cmd_type type;

    union
    {
        struct { size_t cn; size_t ln; size_t maxcn; text_col col; } setfg;
        struct { size_t cn; size_t ln; vec chrs; }                   ins;
        struct { size_t cn; size_t ln; size_t n; }                   del;
        struct { size_t ln; }                                        ins_line;
        struct { size_t ln; }                                        del_line;
    } args;    
};                    

struct text_cur_s
{
    size_t cn1, cn2;
    size_t ln1, ln2;
};

struct text_char_s
{
    char utf8[6];
    text_col bg;
    text_col fg;
};

struct text_buf_s
{
    vec       lines;
    text_cur  cur;
    text_flag flags;
    size_t    scrollx, scrolly;
    size_t    x, y;
    size_t    w, h;
};

text_buf *text_cur_buf;

int text_utf8_len(char utf8);

void text_buf_update_cur(text_buf *b, text_cur *orig);
void text_buf_init(text_buf *b);
void text_buf_kill(text_buf *b);
void text_buf_cmd(text_buf *b, text_cmd *cmd);
void text_cur_cmd(text_cur *cur, text_cmd *cmd);
void text_buf_setfg(text_buf *b, size_t cn, size_t ln, size_t maxcn, text_col col);
void text_buf_ins(text_buf *b, size_t cn, size_t ln, vec *chrs);
void text_buf_del(text_buf *b, size_t cn, size_t ln, size_t n);
void text_buf_del_line(text_buf *b, size_t ln);
void text_buf_ins_line(text_buf *b, size_t ln);

#endif /* TEXT_H */

