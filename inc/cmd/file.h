#if !defined(CMD_FILE_H)
# define CMD_FILE_H
# include "win.h"
# include "vec.h"

void file_clr_win(win *w);

void file_cmd_new     (vec *rtn, vec *args, win *w);
void file_cmd_prev    (vec *rtn, vec *args, win *w);
void file_cmd_next    (vec *rtn, vec *args, win *w);
void file_cmd_load    (vec *rtn, vec *args, win *w);
void file_cmd_save    (vec *rtn, vec *args, win *w);
void file_cmd_assoc   (vec *rtn, vec *args, win *w);
void file_cmd_discard (vec *rtn, vec *args, win *w);
void file_cmd_chdir   (vec *rtn, vec *args, win *w);

void file_load_win(win *w, FILE *f);
void file_save_win(win *w, FILE *f);
#endif
