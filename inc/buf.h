#if !defined(BUF_H)
# define BUF_H
# include "vec.h"
# include "col.h"
# include "chr.h"

typedef struct buf_s buf;

# include "cur.h"

/* Bit flags describing attributes of buffers */
typedef enum
{
    buf_readonly   = 0x01, /* Don't modify this      (not yet implemented) */
    buf_modified   = 0x02, /* This has been modified (not yet implemented) */
    buf_associated = 0x04, /* This buffer is associated with a filename    */
} buf_flags;

/* A representation of some text */
struct buf_s
{
    vec       lines; /* A vector of lines */
    vec       fname; /* The filename associated with this buffer, NULL terminated */
    buf_flags flags; /* The flags of the buffer */
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
