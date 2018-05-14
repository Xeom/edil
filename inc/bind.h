#if !defined(BIND_H)
# define BIND_H
# include "table.h"
# include "inp.h"
# include "win.h"

typedef enum
{
    bind_mode_buf,
    bind_mode_kcd,
    bind_mode_bar
} bind_mode_type;

extern bind_mode_type bind_mode;

extern table bind_kcd;
extern table bind_bar;

typedef void(*bind_fptr)(inp_key k, win *w);

#define RLY_CONCAT(a, b) a ## b
#define CONCAT(a, b) RLY_CONCAT(a, b)

#define BIND_FUNCT(name, code) \
    static void bind_funct_ ## name (inp_key k, win *w); \
    static void bind_funct_ ## name (inp_key k, win *w)  \
    { code; }

#define BIND_TO(name, key) \
    { inp_key k = key; \
      bind_fptr f = bind_funct_ ## name; \
      table_set( tab, &k, &f ); }

#define BIND_SET(name, tab, key) \
    inp_key k = key; \
    bind_fptr f = bind_funct_ ## name; \
    table_set( &bind_ ## tab, &k, &f );

#define BIND_CONSTR(name, tab, key) \
    __attribute__((constructor (300))) void \
    CONCAT(bind_constructor_ ## name ## _,  __COUNTER__) \
    (void) \
    { BIND_SET(name, tab, key) }

#define BIND(name, tab, key, code) \
    BIND_FUNCT(name, code) \
    BIND_CONSTR(name, tab , key)

void bind_ins_flush(void);
void bind_handle_key(inp_key key);
void bind_init(void);
void bind_flush(void);

#endif