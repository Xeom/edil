#if !defined(INDENT_H)
# define INDENT_H
# include "col.h"
# include "chr.h"
# include "buf.h"
# include "vec.h"

extern int      indent_tab_width;
extern char    *indent_tab_text;
extern col_desc indent_tab_col;

/* Print out a tab, as if it were in the column *
 * specified by ind.                            */
void indent_print_tab(size_t ind, FILE *f, col fnt);

/* Get the number of columns taken up by a chr */
int indent_get_width(chr *c, size_t ind);

/* Apply blanks to buffers */
void indent_add_blanks_buf(buf *b);
void indent_add_blanks_line(vec *line, size_t ind);
void indent_add_blanks_chr(vec *line, size_t ind);

#endif
