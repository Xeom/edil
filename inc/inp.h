#if !defined(INP_H)
# define INP_H

typedef enum
{   
    inp_key_back  = 0x07f,
    inp_key_tab   = 0x449,
    inp_key_enter = 0x44a,
    inp_key_esc   = 0x200,
    inp_key_ctrl  = 0x400,
    inp_key_none  = 0x100,
    inp_key_up,
    inp_key_down,
    inp_key_right,
    inp_key_left,
    inp_key_home,
    inp_key_del,
    inp_key_end,
    inp_key_pgup,
    inp_key_pgdn,
    inp_key_insert,
    inp_key_f1, 
    inp_key_f2,
    inp_key_f3,
    inp_key_f4,
    inp_key_f5,
    inp_key_f6,
    inp_key_f7,
    inp_key_f8,
    inp_key_f9,
    inp_key_f10,
    inp_key_f11,
    inp_key_f12
} inp_key;

typedef struct inp_keycode_s inp_keycode;

struct inp_keycode_s
{
    inp_key key;
    char *code;
    char *name;
};

inp_key inp_get_key(char c);

void inp_key_name(inp_key key, char *str, size_t len);

void inp_empty_pipe(void);

void inp_init(void);

void inp_kill(void);

void inp_wait(void);
#endif
