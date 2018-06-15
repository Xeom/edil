#if !defined(BUF_LINE_H)
# define BUF_LINE_H
# include "types.h"

/* Initialize and kill a line. The line should NOT **
 * be locked when these are called.                */
void line_init(line *l);
void line_kill(line *l);

/* Lock and unlock a line using its mutex.       *
 * A line MUST be locked before functions below  *
 * here are called, and then it MUST be unlocked */
void line_lock(line *l);
void line_unlock(line *l);

/* Append the chrs of a line to a vector. */
void line_cpy(line *l, vec *to);

/* Get a pointer to a particular chracter in the *
 * vector of characters                          */
chr *line_chr(line *l, cur c);

/* Get a pointer to the vector of characters */
vec *line_vec(line *l);

/* Get the length of a line */
ssize_t line_len(line *l);

/* Insert characters into a line */
void line_ins(line *l, cur c, vec *chrs);
void line_ins_str(line *l, cur c, char *str);
void line_ins_mem(line *l, cur c, size_t n, chr *mem);

/* Delete characters from a line */
void line_del(line *l, cur c, size_t n);

#endif
