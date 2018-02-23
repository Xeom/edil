#if !defined(COL_H)
# define COL_H
# include <stdio.h>

typedef enum
{
    col_bold    = 0x01,
    col_under   = 0x02,
    col_rev     = 0x04,
    col_blink   = 0x08,
    col_allflgs = 0x0f
} col_flag;

typedef enum
{
    col_black   = 0x00,
    col_red     = 0x01,
    col_green   = 0x02,
    col_yellow  = 0x03,
    col_blue    = 0x04,
    col_magenta = 0x05,
    col_cyan    = 0x06,
    col_white   = 0x07,
    col_bright  = 0x08,
    col_allcols = 0x07,
    col_none    = 0x10,
    col_null    = 0x11
} col_value; 

typedef struct col_s       col;
typedef struct col_delta_s col_delta;
typedef struct col_desc_s  col_desc;

struct col_s
{
    col_flag attr;
    col_value fg;
    col_value bg;
};

struct col_desc_s
{
    col_flag set;
    col_flag del;
    col_flag inv;
    col_value fg;
    col_value bg;
};

col col_update(col c, col_desc d);

void col_print(col c, FILE *f);

#endif
