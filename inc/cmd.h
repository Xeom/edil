#if !defined(CMD_H)
# define CMD_H
# include <stdarg.h>
# include "vec.h"
# include "win.h"

typedef enum
{
    cmd_pipe_chrs,
    cmd_pipe_file,
    cmd_pipe_loc,
    cmd_pipe_none,
    cmd_pipe_cmd
} cmd_pipe_type;


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
#define CMD_FUNCT(_name, _code) \
    static void cmd_funct_ ## _name (vec *args, vec *rtn, win *w); \
    static void cmd_funct_ ## _name (vec *args, vec *rtn, win *w)  \
    {                                                              \
        vec strvecs;                                               \
        vec_init(&_tmpvecs, sizeof(vec *));                        \
        { _code; }                                                 \
        VEC_FOREACH(&_tmpvecs, v, vec_kill(v));                    \
        vec_kill(&_tmpvecs);                                       \
    }

/* Bind a command defined by CMD_FUNCT to a description and full docs, *
 * with the full docs as a string. The command is added to cmd_items.  */
#define CMD_ADD(_name, _desc, _full) \
        static cmd_info info = {         \
            .fptr = cmd_funct_ ## _name, \
            .name = #_name,              \
            .desc = #_desc,              \
            .full = #_full               \
        };                               \
        namevec_item item = {            \
            .name = #_name, .ptr = &info \
        };                               \
        vec_app(&cmd_items, &item);      \
    }


    
#define CMD_ARG(_num, _name) \
    vec *_name = vec_get(args, _num)

#define CMD_ARG_STR(_num, _name) \
    vec *_name = vec_app(&_tmpvecs, NULL);  \
    vec_init(_name, sizeof(char));          \
    chr_to_str(vec_get(args, _num), _name); \
    vec_app(_name, "\0");

#define CMD_ARG_SCAN(_num, _fmt, ...) \
    chr_scan(vec_get(args, _num), _fmt, __VA_ARGS__)

#define CMD_ARG_PARSE(_num, _fmt, _var) \
    if (CMD_ARG_SCAN(_num, _fmt, _var) != 1)             \
    {                                                    \
        CMD_RTN("err: Could not parse argument " #_num); \
        return;                                          \
    }

#define CMD_RTN(_str) \
    chr_from_str(rtn, _str)

#define CMD_RTN_VEC(_vec) \
    chr_from_vec(rtn, _vec)

#define CMD_RTN_CHR(_vec) \
    vec_cpy(rtn, _vec)

#define CMD_RTN_FMT(_fmt, ...) \
    chr_format(rtn, _fmt, __VA_ARGS__)

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
    if (vec_len(args) < num + 1)                             \
    {                                                        \
        CMD_RTN_FMT(                                         \
            "err: This command takes %d argument%s minimum", \
            _num, (_num == 1) ? "" : "s"                     \
        );                                                   \
        return;                                              \
    }

#define CMD_NARGS vec_len(args) - 1

extern vec cmd_items;

void cmd_init(void);
void cmd_kill(void);

void cmd_run(vec *args, vec *rtn, win *w);

void cmd_parse(vec *args, vec *chrs, size_t ind);

void cmd_log(vec *chrs, int iscmd);
#endif
