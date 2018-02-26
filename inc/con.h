#if !defined(CON_H)
# define CON_H
# include "inp.h"

void con_init(void);

void con_ins_flush(void);

void con_flush(void);

void con_ins(inp_key key);

void con_handle(inp_key key);

#endif
