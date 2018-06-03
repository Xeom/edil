#if !defined(BIND_H)
# define BIND_H
# include "table.h"
# include "inp.h"
# include "win.h"

typedef enum
{
    bind_mode_buf,
    bind_mode_kcd,
    bind_mode_bar,
    bind_mode_mov,
    bind_mode_none
} bind_mode_type;

typedef struct bind_info_s      bind_info;
typedef struct bind_mode_info_s bind_mode_info;

# include "namevec.h"

/* This is a function that bound to a particular keystroke, *
 * and called whenever that key is pressed.                 */
typedef void(*bind_fptr)(win *w, inp_key k);

/* This structure contains information about a particular *
 * 'bind'. This is an action that is performed when a key *
 * is pressed.                                            *
 * A namevec containing all bind_infos is stored as       *
 * bind_all.                                              */
struct bind_info_s
{
    bind_fptr fptr; /* A pointer to the function to call      */
    char *desc;     /* A description of the binding           */
    char *name;     /* The name of the binding, used by remap */
};

struct bind_mode_info_s
{
    char  *name;                  /* The name of the mode              */
    bind_mode_type mode;          /* The identifier of the mode        */

    table *keytable;              /* Mapping keypresses to bind_infos  */

    /* These are function pointers that are expected to, when called, *
     * add all the necessary bind_infos to keytable and bind_all.     */
    void (*initf)(table *t);      /* Function called on edil init      */

    /* These two function pointers provide general handling of keys for *
     * when it is difficult to map every possible key.                  *
     * keyf also provides translation from inp_keys to unicode chrs.    *
     * Both can be NULL, and if so, they are ignored. Bindings made in  *
     * keytable override calls to both of these functions.              */
    void (*insf)(win *, vec *);   /* Called with string of typed chrs  */
    void (*keyf)(win *, inp_key); /* Called with single non-bound keys */
};

/* The current mode */
extern bind_mode_type bind_mode;
extern vec bind_all;

/* These are definitions that are used to mount functions */

/* This is used to define a new bindable action */
#define BIND_FUNCT(name, code) \
    static void bind_funct_ ## name (win *w, inp_key k); \
    static void bind_funct_ ## name (win *w, inp_key k)  \
    { code; }

/* This define will set up a bindable action in an init function, *
 * and assumed that the appropriate keytable is named tab.        */
#define BIND_TO(_name, key, _desc) { \
        inp_key k = key; \
        static bind_info info = { .fptr = bind_funct_ ## _name, \
                           .name = #_name, \
                           .desc = #_desc }; \
        namevec_item item = { .name = #_name, \
                              .data.bind = &info }; \
        vec_app(&bind_all, &item); \
        if (k != inp_key_none) table_set(tab,  &k, &info); \
    } \

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

/* Unmap and remap bindings according to their names */
int bind_remap(vec *chrmode, inp_key k, vec *chrbind);
int bind_unmap(vec *chrmode, inp_key k);

/* Set up and kill the binding system */
void bind_init(void);
void bind_kill(void);


void bind_print(FILE *stream);
void bind_print_table(table *t, FILE *stream);

#endif