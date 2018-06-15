#if !defined(BIND_H)
# define BIND_H
# include "types.h"

/* The current mode */
extern bind_mode_type bind_mode;
extern vec bind_all;

/* These are definitions that are used to mount functions */

/* This is used to define a new bindable action */
#define BIND_FUNCT(_name, _code) \
    static void bind_funct_ ## _name (win *w, inp_key k); \
    static void bind_funct_ ## _name (win *w, inp_key k)  \
    { _code; }

#define BIND_ADD(_name, _desc) {               \
        static bind_info info = {              \
            .fptr = bind_funct_ ## _name,      \
            .name = #_name, .desc = #_desc     \
        };                                     \
        namevec_item item = {                  \
            .name = #_name, .ptr = &info       \
        };                                     \
        vec_app(&bind_all, &item);             \
    }

#define BIND_MAP(_mode, _name, _key) { \
        bind_remap_str(bind_mode_ ## _mode, _key, #_name); \
    }

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

/* This is called by the ui after a long string of characters. *
 * It tells the binding system that it needs to stop buffering *
 * actions and do everything.                                  */
void bind_flush(void);

/* This function is called every time a key is pressed, the *
 * key is handled appropriately                             */
void bind_handle_key(inp_key key);

/* Get modes and bindings from their names */
bind_mode_type bind_mode_get(vec *chrname);
bind_info *bind_info_get(vec *chrname);
bind_mode_type bind_mode_get_str(char *str);
bind_info *bind_info_get_str(char *str);

bind_mode_info *bind_info_curr(void);

/* Unmap and remap bindings according to their names */
int bind_remap(vec *chrmode, inp_key k, vec *chrbind);
int bind_unmap(vec *chrmode, inp_key k);
int bind_remap_str(bind_mode_type mode, inp_key k, char *str);

/* Set up and kill the binding system */
void bind_init(void);
void bind_kill(void);

/* Print out the edil -b table */
void bind_print(FILE *stream);
void bind_print_table(bind_mode_info *mode, FILE *stream);

#endif
