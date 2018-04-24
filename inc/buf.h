#if !defined(BUF_H)
# define BUF_H
# include "vec.h"
# include "col.h"
# include "chr.h"

typedef struct buf_s buf;

# include "cur.h"
# include "file.h"

/* Bit flags describing attributes of buffers */
typedef enum
{
    buf_readonly   = 0x01, /* Don't modify this                            */
    buf_modified   = 0x02, /* This has been modified                       */
    buf_nofile     = 0x04, /* Buffer cannot be associated with a filename  */
} buf_flags;

/* A representation of some text */
struct buf_s
{
    vec       lines; /* A vector of lines */
    file      finfo;
    buf_flags flags; /* The flags of the buffer */
};

void buf_init(buf *b);

void buf_kill(buf *b);

void buf_setcol(buf *b, cur loc, size_t n, col_desc col);

void buf_ins(buf *b, cur loc, chr *chrs, size_t n);

void buf_del(buf *b, cur loc, size_t n);

void buf_clr(buf *b);

void buf_ins_nl(buf *b, cur loc);

void buf_del_nl(buf *b, cur loc);

void buf_ins_line(buf *b, cur loc);

void buf_del_line(buf *b, cur loc);

size_t buf_len(buf *b);

ssize_t buf_line_len(buf *b, cur loc);

vec *buf_line(buf *b, cur loc);

chr *buf_chr(buf *b, cur loc);

cur buf_last_cur(buf *b);

void buf_ins_buf(buf *b, cur *c, buf *other, cur loc, cur end);

#endif
