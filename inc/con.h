#if !defined(CON_H)
# define CON_H
# include "inp.h"

typedef enum
{
    con_mode_buf,
    con_mode_kcd,
    con_mode_bar
} con_mode_type;

extern int con_alive;
extern con_mode_type con_mode;

void con_init(void);

void con_kill(void);

int con_is_typable(inp_key key);

void con_ins_flush(void);

void con_flush(void);

void con_ins(inp_key key);

void con_handle(inp_key key);

#endif
