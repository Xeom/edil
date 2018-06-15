#if !defined(COL_H)
# define COL_H
# include <stdio.h>
# include "types.h"

extern col col_default;

col col_update(col c, col_desc d);

void col_print(col c, FILE *f);

int col_parse(col *c, char **str);

void col_parse_string(col c, vec *chrs, char *str);

#endif
