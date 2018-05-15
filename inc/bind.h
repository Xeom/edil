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

typedef struct bind_info_s bind_info;

typedef void(*bind_fptr)(inp_key k, win *w);

struct bind_info_s
{
    bind_fptr fptr;
    char *desc;
};

extern bind_mode_type bind_mode;


#define RLY_CONCAT(a, b) a ## b
#define CONCAT(a, b) RLY_CONCAT(a, b)

#define BIND_FUNCT(name, code) \
    static void bind_funct_ ## name (inp_key k, win *w); \
    static void bind_funct_ ## name (inp_key k, win *w)  \
    { code; }

#define BIND_TO(name, key, _desc) \
    { inp_key k = key; \
      bind_info info = { .fptr = bind_funct_ ## name, \
                         .desc = #_desc }; \
      table_set( tab, &k, &info ); }

void bind_ins_flush(void);
void bind_handle_key(inp_key key);

void bind_init(void);
void bind_kill(void);

void bind_flush(void);

void bind_print(FILE *stream);
void bind_print_table(table *t, FILE *stream);

#endif