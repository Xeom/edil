#if !defined(BIND_CURBIND_H)
# define BIND_CURBIND_H

#define _BIND_MAP_UDLR(mode, bind, u, d, l, r) \
    BIND_MAP(mode, bind ## _u, u); \
    BIND_MAP(mode, bind ## _d, d); \
    BIND_MAP(mode, bind ## _l, l); \
    BIND_MAP(mode, bind ## _r, r);

#define BIND_MAP_UDLR(mode, bind, ...) \
    _BIND_MAP_UDLR(mode, bind, __VA_ARGS__)

#define BIND_KEYS_UDLR(arg) \
    inp_key_up   | (arg), inp_key_down  | (arg), \
    inp_key_left | (arg), inp_key_right | (arg)
    
void bind_curbind_init(void);

#endif