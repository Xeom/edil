#if !defined(CLI_H)
# define CLI_H

# include "text.h"

typedef enum
{   
    cli_key_back  = 0x07f,
    cli_key_tab   = 0x449,
    cli_key_enter = 0x44a,
    cli_key_esc   = 0x200,
    cli_key_ctrl  = 0x400,
    cli_key_none  = 0x100,
    cli_key_up,
    cli_key_down,
    cli_key_right,
    cli_key_left,
    cli_key_home,
    cli_key_del,
    cli_key_end,
    cli_key_pgup,
    cli_key_pgdn,
    cli_key_insert,
    cli_key_f1, 
    cli_key_f2,
    cli_key_f3,
    cli_key_f4,
    cli_key_f5,
    cli_key_f6,
    cli_key_f7,
    cli_key_f8,
    cli_key_f9,
    cli_key_f10,
    cli_key_f11,
    cli_key_f12
} cli_key;

typedef struct cli_keycode_s cli_keycode;

struct cli_keycode_s
{
    cli_key key;
    char code[16];
};

extern int cli_h, cli_w;
extern int cli_alive;
extern vec cli_keycodes;

void cli_init(void);
void cli_kill(void);
cli_key cli_get_key(char chr);
void cli_goto(size_t cn, size_t ln);
void cli_fg(text_col col);
void cli_line_here(text_buf *b, size_t cn, size_t ln, size_t len);
void cli_line(text_buf *b, size_t cn, size_t ln);
void cli_lines_after(text_buf *b, size_t ln);

#endif /* CLI_H */

