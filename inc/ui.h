#if !defined(CON_H)
# define CON_H
# include "inp.h"

typedef enum
{
    ui_mode_buf,
    ui_mode_kcd,
    ui_mode_bar
} ui_mode_type;

extern int ui_alive;
extern ui_mode_type ui_mode;

void ui_init(void);

void ui_kill(void);

int ui_is_typable(inp_key key);

void ui_ins_flush(void);

void ui_flush(void);

void ui_ins(inp_key key);

void ui_handle(inp_key key);

#endif
