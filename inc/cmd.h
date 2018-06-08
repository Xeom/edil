#if !defined(CMD_H)
# define CMD_H
# include <stdarg.h>
# include "vec.h"
# include "win.h"
# include "namevec.h"

typedef struct cmd_info_s cmd_info;

struct cmd_info_s
{
    void (*fptr)(vec *, vec *, win *);
    char *name;
    char *desc;
    char *full;
};

/* Define code as a static function with a particular command name.         *
 * The code is run the variables win *w, vec *args, vec *rtn as parameters. */
#define CMD_FUNCT(_name, ...) \
    static void  cmd_funct_ ## _name (vec *args, vec *rtn, win *w);                \
    static void _cmd_funct_ ## _name (vec *args, vec *rtn, win *w, vec *_tmpvecs); \
    static void  cmd_funct_ ## _name (vec *args, vec *rtn, win *w)                 \
    {                                                                              \
        vec _tmpvecs;                                                              \
        vec_init(&_tmpvecs, sizeof(vec *));                                        \
        _cmd_funct_ ## _name (args, rtn, w, &_tmpvecs);                            \
        VEC_FOREACH(&_tmpvecs, vec **, vptr,                                       \
            vec_kill(*vptr);                                                       \
        );                                                                         \
        vec_kill(&_tmpvecs);                                                       \
    }                                                                              \
    static void _cmd_funct_ ## _name (vec *args, vec *rtn, win *w, vec *_tmpvecs)  \
    { __VA_ARGS__ }

/* Bind a command defined by CMD_FUNCT to a description and full docs, *
 * with the full docs as a string. The command is added to cmd_items.  */
#define CMD_ADD(_name, _desc, _full) \
    {                                    \
        static cmd_info info = {         \
            .fptr = cmd_funct_ ## _name, \
            .name = #_name,              \
            .desc = #_desc,              \
            .full = _full                \
        };                               \
        namevec_item item = {            \
            .name = #_name, .ptr = &info \
        };                               \
        vec_app(&cmd_items, &item);      \
    }

/* Create a vector that is automatically killed after the command */
#define CMD_TMP_VEC(_name, _type) \
    vec *_name = malloc(sizeof(vec));         \
    *(vec **)vec_app(_tmpvecs, NULL) = _name; \
    vec_init(_name, sizeof(_type));

/* Get arguments as chr vecs */
#define CMD_ARG(_num, _name) \
    vec *_name = vec_get(args, _num)

/* Get arguments as char vecs */
#define CMD_ARG_STR(_num, _name) \
    CMD_TMP_VEC(_name, char)                \
    chr_to_str(vec_get(args, _num), _name); \
    vec_app(_name, "\0");

/* chr_scan arguments */
#define CMD_ARG_SCAN(_num, _fmt, ...) \
    chr_scan(vec_get(args, _num), _fmt, __VA_ARGS__)

/* chr_scan arguments with error check */
#define CMD_ARG_PARSE(_num, _fmt, _var) \
    if (CMD_ARG_SCAN(_num, _fmt, _var) != 1)             \
    {                                                    \
        CMD_RTN("err: Could not parse argument " #_num); \
        return;                                          \
    }

/* Return a char * string */
#define CMD_RTN(_str) \
    chr_from_str(rtn, _str)

/* Return a char vector */
#define CMD_RTN_VEC(_vec) \
    chr_from_vec(rtn, _vec)

/* Return a chr vector */
#define CMD_RTN_CHR(_vec) \
    vec_cpy(rtn, _vec)

/* Return a formatted string */
#define CMD_RTN_FMT(_fmt, ...) \
    chr_format(rtn, _fmt, __VA_ARGS__)

#define CMD_ERR(_str) \
    do { CMD_RTN("err: " _str); return; } while (0)

#define CMD_ERR_FMT(_fmt, ...) \
    do { CMD_RTN_FMT("err: " _fmt, __VA_ARGS__); return; } while (0)

/* Check the number of arguments */
#define CMD_MAX_ARGS(_num) \
    if (vec_len(args) > _num + 1)                                  \
    {                                                              \
        CMD_RTN_FMT(                                               \
            "err: This command takes up to %d argument%s maximum", \
            _num, (_num == 1) ? "" : "s"                           \
        );                                                         \
        return;                                                    \
    }

#define CMD_MIN_ARGS(_num)                                   \
    if (vec_len(args) < _num + 1)                             \
    {                                                        \
        CMD_RTN_FMT(                                         \
            "err: This command takes %d argument%s minimum", \
            _num, (_num == 1) ? "" : "s"                     \
        );                                                   \
        return;                                              \
    }

/* Check if an argument is a specific string */
#define CMD_ARG_IS(_num, _str) \
    (chr_cmp_str(vec_get(args, _num), _str) == 0)

/* Get number of arguments */
#define CMD_NARGS (vec_len(args) - 1)

extern vec cmd_items;

void cmd_init(void);
void cmd_kill(void);

void cmd_run(vec *args, vec *rtn, win *w);

void cmd_parse(vec *args, vec *chrs, size_t ind);

void cmd_log(vec *chrs, int iscmd);

void cmd_print_all(FILE *stream);
void cmd_print_info(FILE *stream, cmd_info *info);
void cmd_print_pre(FILE *stream, cmd_info *info);
#endif
