#if !defined(INDENT_H)
# define INDENT_H
# include "col.h"
# include "chr.h"
# include "buf/buf.h"
# include "vec.h"

typedef enum
{
    indent_spacify   = 0x01,
    indent_auto      = 0x02,
    indent_skipblank = 0x04,
    indent_trim      = 0x08
} indent_flag;

extern indent_flag indent_mode;
extern int         indent_tab_width;
extern int         indent_lvl_width;
extern char       *indent_tab_text;
extern col_desc    indent_tab_col;

/* Print out a tab, as if it were in the column *
 * specified by ind.                            */
void indent_print_tab(size_t ind, FILE *f, col fnt);

/* Get the number of columns taken up by a chr */
int indent_get_width(chr *c, size_t ind);

/* Apply blanks to buffers */
void indent_add_blanks_buf(buf *b);
void indent_add_blanks_line(vec *line, size_t ind);
void indent_add_blanks_chr(vec *line, size_t ind);

void indent_set_tab_width(size_t width);

void indent_ins_tab(buf *b, cur c);

int    indent_is_blank(buf *b, cur c);
size_t indent_get_depth(buf *b, cur c);
void   indent_set_depth(buf *b, cur c, size_t depth);
void   indent_trim_end(buf *b, cur c);

cur indent_incr_depth(buf *b, cur c);
cur indent_decr_depth(buf *b, cur c);
cur indent_auto_depth(buf *b, cur c);
#endif
