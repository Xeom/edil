#if !defined(BUF_H)
# define BUF_H
# include "vec.h"
# include "col.h"
# include "chr.h"

typedef struct buf_s buf;

# include "cur.h"

typedef enum
{
    buf_readonly = 0x01,
    buf_modified = 0x02
} buf_flags;

struct buf_s
{
    vec       lines;
    buf_flags flags;
};

void buf_init(buf *b);

void buf_kill(buf *b);

void buf_setcol(buf *b, cur loc, size_t n, col_desc col);

void buf_ins(buf *b, cur loc, chr *chrs, size_t n);

void buf_del(buf *b, cur loc, size_t n);

void buf_ins_line(buf *b, cur loc);

void buf_del_line(buf *b, cur loc);

size_t buf_len(buf *b);

size_t buf_line_len(buf *b, cur loc);

#endif
