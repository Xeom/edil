#if !defined(FILES_H)
# define FILES_H
# include <stdio.h>
# include "text.h"

void files_load_query(text_buf *b);

void files_load_confirm(char *str);

void files_load(text_buf *b, FILE *f);

#endif /* FILES_H */
