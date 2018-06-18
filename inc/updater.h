#if !defined(UPDATER_H)
# define UPDATER_H
# include "types.h"

void updater_init(void);

void updater_start(updater *u);
void updater_end(updater *u);

void updater_del_buf(buf *b);

void updater_send_to(updater *u);

void updater_after(buf *b, cur c);

void updater_line(buf *b, cur c);

#endif
