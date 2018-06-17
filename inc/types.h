#if !defined(TYPES_H)
# define TYPES_H

# include <pthread.h>
# include <stdio.h>
# include <unistd.h>
# include <stddef.h>

/* * * * * * * * *
 * TYPEDEF TIME! *
 * * * * * * * * */

typedef struct bar_s            bar;            /* Bar at base of windows */
typedef struct bind_info_s      bind_info;      /* Function bound to key  */
typedef struct bind_mode_info_s bind_mode_info; /* UI Mode                */
typedef struct buf_s            buf;            /* Text and metadata      */
typedef struct chr_s            chr;            /* Utf-8 char with font   */
typedef struct circvec_s        circvec;        /* A circular buffer      */
typedef struct cmd_info_s       cmd_info;       /* Function bound as cmd  */
typedef struct cur_s            cur;            /* Position in buffer     */
typedef struct file_s           file;           /* File or pipe etc.      */
typedef struct inp_keycode_s    inp_keycode;    /* Key escape code        */
typedef struct line_s           line;           /* A line of text         */
typedef struct namevec_item_s   namevec_item;   /* Maps a name to a void* */
typedef struct table_s          table;          /* A hashtable container  */
typedef struct text_s           text;           /* A series of lines      */
typedef struct vec_s            vec;            /* A vector container     */
typedef struct win_s            win;            /* Displays a buffer      */

/* * * * * * * *
 * CONTAINERS  *
 * * * * * * * */

struct vec_s
{
    char *data;      /* The stored data, can be NULL.      */
    size_t capacity; /* Capacity usage and width in bytes. */
    size_t usage;    /* The number of bytes used.          */
    size_t width;    /* The number of bytes per item.      */
};

/* Key-value pairs are stored in the table *
 * as an array of blocks:                  *
 *                                         *
 * struct                                  *
 * {                                       *
 *     uint64_t hash;                      *
 *     char key[keywidth];                 *
 *     char value[valwidth];               *
 * }                                       */
struct table_s
{
    size_t capacity; /* The number of blocks memory is allocated for */
    size_t usage;    /* The number of blocks used                    */
    size_t valwidth; /* The sizeof the values being stored           */
    size_t keywidth; /* The sizeof the keys being store              */
    size_t blkwidth; /* The total size of a block                    */
    char  *data;     /* The array of blocks                          */
};

struct circvec_s
{
    vec v;
    size_t ins, del;
    unsigned int empty : 1;
};

struct namevec_item_s
{
    char *name;
    void *ptr;
};

/* * * * *
 * FILES *
 * * * * */

typedef enum
{
    file_cr    = 0x01,
    file_pipe  = 0x02,
    file_eofnl = 0x04
} file_flags;

struct file_s
{
    vec fname;
    vec dirname;
    vec basename;
    FILE *fptr;
    file_flags flags;
};

/* * * * * *
 * COLOURS *
 * * * * * */

typedef enum
{
    col_bold    = 0x01,
    col_under   = 0x02,
    col_rev     = 0x04,
    col_blink   = 0x08,
    col_allflgs = 0x0f
} col_flag;

typedef enum
{
    col_black   = 0x00,
    col_red     = 0x01,
    col_green   = 0x02,
    col_yellow  = 0x03,
    col_blue    = 0x04,
    col_magenta = 0x05,
    col_cyan    = 0x06,
    col_white   = 0x07,
    col_bright  = 0x08,
    col_allcols = 0x07,
    col_none    = 0x10,
    col_null    = 0x11
} col_value;

typedef struct col_s      col;
typedef struct col_desc_s col_desc;

struct col_s
{
    col_flag attr;
    col_value fg;
    col_value bg;
};

struct col_desc_s
{
    col_flag set;
    col_flag del;
    col_flag inv;
    col_value fg;
    col_value bg;
};

/* * * * * * * * *
 * STORING TEXT  *
 * * * * * * * * */

/* Represents a position within a buffer */
struct cur_s
{
    ssize_t cn, ln;
};

/* Represents one utf-8 character */
struct chr_s
{
    char utf8[6]; /* The utf-8 encoded character */
    col  fnt;     /* The colour/attributes of the character */
};

/* Represents a line of utf-8 characters */
struct line_s
{
    pthread_mutex_t lock;
    vec chrs;
};

/* Represents a load of utf-8 characters */
struct text_s
{
    pthread_mutex_t lock;
    vec lines;
};

/* Bit flags describing attributes of buffers */
typedef enum
{
    buf_readonly   = 0x01, /* Don't modify this                            */
    buf_modified   = 0x02, /* This has been modified                       */
    buf_nofile     = 0x04, /* Buffer cannot be associated with a filename  */
    buf_nokill     = 0x08  /* Buffer cannot be killed */
} buf_flags;

/* Contains and manages a text structure */
struct buf_s
{
    text t;
    file finfo;
    vec  name;
    cur  prihint;
    buf_flags flags;
};

/* * * * * * * *
 * UI ELEMENTS *
 * * * * * * * */

typedef enum
{
    inp_key_esc       = 0x200,
    inp_key_ctrl      = 0x400,
    inp_key_none      = 0x100,
    inp_key_up,
    inp_key_down,
    inp_key_right,
    inp_key_left,
    inp_key_home,
    inp_key_end,
    inp_key_pgup,
    inp_key_pgdn,
    inp_key_insert,
    inp_key_shifttab,
    inp_key_shiftdel,
    inp_key_shiftback,
    inp_key_tab,
    inp_key_del,
    inp_key_back,
    inp_key_enter,
    inp_key_pastebegin,
    inp_key_pasteend,
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

struct inp_keycode_s
{
    inp_key key;
    char *code;
    char *name;
};

struct bar_s
{
    vec  *typed;
    vec   prompt;
    char *format;
    int ind, scrind;

    circvec scrollback;

    void (*cb)(win *w, vec *chrs);

    win *w;
};

struct win_s
{
    /* Dimensions */
    ssize_t scrx, scry;
    ssize_t rows, cols;
    ssize_t xpos, ypos;

    /* Text and cursors */
    buf   *b;
    cur pri, sec;

    /* Bar at the bottom */
    bar basebar;
};


/* * * * * *
 * BINDING *
 * * * * * */

typedef enum
{
    bind_mode_buf,
    bind_mode_kcd,
    bind_mode_bar,
    bind_mode_mov,
    bind_mode_lng,
    bind_mode_none
} bind_mode_type;

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

    /* These two function pointers provide general handling of keys for *
     * when it is difficult to map every possible key.                  *
     * keyf also provides translation from inp_keys to unicode chrs.    *
     * Both can be NULL, and if so, they are ignored. Bindings made in  *
     * keytable override calls to both of these functions.              */
    void (*insf)(win *, vec *);   /* Called with string of typed chrs  */
    void (*keyf)(win *, inp_key); /* Called with single non-bound keys */
};

struct cmd_info_s
{
    void (*fptr)(vec *, vec *, win *);
    char *name;
    char *desc;
    char *full;
};

typedef enum
{
    indent_spacify   = 0x01,
    indent_auto      = 0x02,
    indent_skipblank = 0x04,
    indent_trim      = 0x08
} indent_flag;


#endif
