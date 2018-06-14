#if !defined(BUF_H)
# define BUF_H
# include "buf/text.h"
# include "chr.h"

typedef struct buf_s buf;

# include "file.h"

/* Bit flags describing attributes of buffers */
typedef enum
{
    buf_readonly   = 0x01, /* Don't modify this                            */
    buf_modified   = 0x02, /* This has been modified                       */
    buf_nofile     = 0x04, /* Buffer cannot be associated with a filename  */
    buf_nokill     = 0x08  /* Buffer cannot be killed */
} buf_flags;

struct buf_s
{
    text t;
    file finfo;
    vec  name;
    cur  prihint;
    buf_flags flags;
};

void buf_init(buf *b);
void buf_kill(buf *b);

void buf_ins(buf *b, cur loc, vec *chrs);
void buf_ins_str(buf *b, cur loc, char *str);
void buf_ins_mem(buf *b, cur loc, size_t n, chr *mem);

void buf_del(buf *b, cur loc, size_t n);

void buf_clr(buf *b);

void buf_ins_nl(buf *b, cur loc);
void buf_del_nl(buf *b, cur loc);

chr buf_chr(buf *b, cur loc);

void buf_ins_from(buf *b, cur c, buf *oth, cur loc, cur end);

void buf_set_name(buf *b, char *name);
char *buf_get_name(buf *b);

#endif
