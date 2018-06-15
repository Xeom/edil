#include <sys/types.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>

#include "container/table.h"
#include "bar.h"
#include "inp.h"
#include "bind.h"
#include "file.h"
#include "win.h"
#include "cmd.h"
#include "ui.h"

#include "cmd/conf.h"

static char *conf_default_files[] =
{
    ".edil.conf",
    ".edil",
    ".edil.remap",
    ".config/edil",
    ".config/edil.conf",
    ".config/edil.remap"
};

void conf_run_file(file *f, win *w)
{
    vec line;
    vec_init(&line, sizeof(chr));

    while (!file_ended(f))
    {
        chr *first;

        file_read_line(f, &line);

        if (!vec_len(&line)) continue;

        first = vec_get(&line, 0);

        if (strcmp(first->utf8, "#") != 0)
            ui_cmd_cb(w, &line);
    }

    vec_kill(&line);
}

void conf_run_file_name(char *name, win *w)
{
    file f;
    vec chrname;
    vec_init(&chrname, sizeof(chr));

    chr_from_str(&chrname, name);

    file_init(&f);
    file_assoc(&f, &chrname);

    if (file_exists(&f))
    {
        file_open(&f, "r");
        conf_run_file(&f, w);
    }

    file_kill(&f);
    vec_kill(&chrname);
}

void conf_run_default_files(win *w)
{
    size_t ind, num;
    struct passwd *info;
    char  *home;

    info = getpwuid(getuid());
    home = info->pw_dir;

    num = sizeof(conf_default_files)/sizeof(char *);
    for (ind = 0; ind < num; ++ind)
    {
        vec  name;

        vec_init(&name, sizeof(char));

        vec_str(&name, home);
        vec_str(&name, "/");
        vec_str(&name, conf_default_files[ind]);
        vec_app(&name, "\0");

        conf_run_file_name(vec_first(&name), w);

        vec_kill(&name);
    }
}

CMD_FUNCT(conffile,
    CMD_MIN_ARGS(1);

    size_t argind;

    for (argind = 1; argind < CMD_NARGS; ++argind)
    {
        file f;

        CMD_ARG(argind, path);
        file_init(&f);

        if (file_assoc(&f, path) == -1)
        {
            file_kill(&f);
            CMD_ERR_FMT(
                "Could not parse path - [%d] %s",
                errno, strerror(errno)
            );
        }

        if (!file_exists(&f))
        {
            file_kill(&f);
            CMD_ERR_FMT("file '%s' does not exist", file_name(&f));
        }

        if (file_open(&f, "r") == -1)
        {
            file_kill(&f);
            CMD_ERR_FMT(
                "Could not open file - [%d] %s",
                errno, strerror(errno)
            );
        }

        conf_run_file(&f, w);

        CMD_RTN_FMT("%sRan '%s'", (argind > 1) ? ", " : "", file_base(&f));

        file_kill(&f);
    }
)

CMD_FUNCT(remap,
    CMD_MIN_ARGS(3);
    CMD_MAX_ARGS(3);

    int keyval;
    char name[64];

    CMD_ARG(1, mode);
    CMD_ARG_PARSE(2, "%x", &keyval);
    CMD_ARG(3, bind);

    CMD_ARG_STR(1, modestr);
    CMD_ARG_STR(3, bindstr);

    inp_key_name(keyval, name, sizeof(name));

    if (bind_remap(mode, keyval, bind) == -1)
        CMD_ERR("Could not remap key");

    CMD_RTN_FMT(
        "Mapped (%s) to '%s' for %s mode",
        name, vec_first(bindstr), vec_first(modestr)
    );
)

CMD_FUNCT(unmap,
    CMD_MIN_ARGS(2);
    CMD_MAX_ARGS(2);

    int keyval;
    char name[64];

    CMD_ARG(1, mode);
    CMD_ARG_PARSE(2, "%x", &keyval);

    CMD_ARG_STR(1, modestr);

    inp_key_name(keyval, name, sizeof(name));

    if (bind_unmap(mode, keyval) == -1)
        CMD_ERR("Could not unmap key");

    CMD_RTN_FMT("Unmapped (%s) for %s mode", name, vec_first(modestr));
)

CMD_FUNCT(translate,
    CMD_MIN_ARGS(2);
    CMD_MAX_ARGS(2);

    int fromval, toval;
    char fromname[64], toname[64];

    CMD_ARG_PARSE(1, "%x", &fromval);
    CMD_ARG_PARSE(2, "%x", &toval);

    table_set(&inp_keytranslate, &fromval, &toval);

    inp_key_name(fromval, fromname, sizeof(fromname));
    inp_key_name(toval,   toname,   sizeof(toname));

    CMD_RTN_FMT("Translating (%s) keys to (%s) keys", fromname, toname);
)

CMD_FUNCT(basebar,
    bar *base;

    CMD_MIN_ARGS(0);
    CMD_MAX_ARGS(1);

    base = &(w->basebar);

    if (CMD_NARGS)
    {
        CMD_ARG_STR(1, fmt);
        bar_set_format(base, vec_first(fmt));
    }

    CMD_RTN_FMT("Basebar format: '%s'", base->format);
)

void cmd_conf_init(void)
{
    CMD_ADD(conffile,
        Load a config file,
        "Configuration files are just lists of commands, one on each line,\n"
        "that are run by Edil. The commands are run just as if they were\n"
        "typed after pressing `Ctrl+X`. Blank lines, and those starting\n"
        "with a `#` symbol are ignored.\n\n"

        "This command can be passed multiple paths as arguments, and will\n"
        "run the commands contained in each one. It requires at least one\n"
        "argument.\n\n"

        "Commands in `~/.edil`, `~/.edil.conf`, `~/.edil.remap`,\n"
        "`~/.config/edil`, `~/.config/edil.conf`, and `~/config/edil.remap`\n"
        "are run by default when Edil starts up.\n\n"
    );

    CMD_ADD(remap,
        Remap a key,
        "Remaps a key to a new binding for a specific mode. The\n"
        "[unmap](#unmap-command) command removes one of these mappings.\n"
        "The `remap` command takes three arguments, the first is the three\n"
        "letter name of the mode being affected, e.g. `buf`, `bar`, or\n"
        "`mov`.\n\n"

        "The second argument is the key being rebound, as a hexadecimal\n"
        "keycode. These keycodes can be found by pressing `Ctrl+K` in edil\n"
        "and entering keycode mode. When in this mode, every keypress types\n"
        "the relevant key name and code as a hexadecimal number. Internally\n"
        "these values are stored in the `inp_key` enum, defined in\n"
        "the [inp header](/inc/inp.h).\n\n"

        "The third argument is the bind to remap. e.g. `cur_mv_l`,\n"
        "`cmd_goto`, or `mode_mov`. The bindings currently used can be\n"
        "viewed the [doc/keys.md](/doc/keys.md) file, or can be viewed by\n"
        "running the command `edil --binds`. They are the third column.\n\n"

        "Once a binding is remapped, pressing the key associated with it,\n"
        "while in the correct mode, will run the associated bind. For\n"
        "example:\n"
        "```\n"
        "remap mov 068 cur_mv_l\n"
        "remap mov 06a cur_mv_d\n"
        "remap mov 06b cur_mv_u\n"
        "remap mov 06c cur_mv_r\n"
        "```\n"
        "will remap the `h j k l` keys to move the cursor in movement mode,\n"
        "vim style!\n"
    );

    CMD_ADD(unmap,
        Unmap a key,
        "This command removes the binding from a key in a specific mode.\n"
        "It is the opposite of the [remap](#remap-command) command. It takes\n"
        "only two arguments. The first is the three letter name of the mode\n"
        "that a mapping is being removed from, e.g. `kcd`, or `buf`.\n\n"

        "The last argument is the key being unbound, as a hexadecimal\n"
        "keycode. These keycodes can be found by pressing `Ctrl+K` in edil\n"
        "and entering keycode mode. When in this mode, every keypress types\n"
        "the relevant key name and code as a hexadecimal number. Internally\n"
        "these values are stored in the `inp_key` enum, defined in\n"
        "the [inp header](/inc/inp.h).\n\n"

        "Unbinding a key causes nothing to happen when it is pressed while\n"
        "edil is in the relevant mode. For example,\n"
        "```\n"
        "unmap mov 64b\n"
        "```\n"
        "will unmap the shortcut of the `quit` command from `Ctrl+Esc+K`.\n"

    );

    CMD_ADD(translate,
        Translate a keypress,
        "This command adds a pair of `inp_key`s to the `inp_keytranslate`\n"
        "table. When an input key is received by the input system, it is\n"
        "passed through this table, which for example, turns `Ctrl+I` to\n"
        "`inp_key_tab`. Making changes to this table can be useful if your\n"
        "terminal maps different keys to different codes.\n\n"

        "The first argument is the keycode being mapped from, and the second\n"
        "is the keycode being mapped to. Both are hexadecimal numbers.\n"
        "These keycodes can be found by pressing `Ctrl+K` in edil and\n"
        "entering keycode mode. When in this mode, every keypress types\n"
        "the relevant key name and code as a hexadecimal number. Internally\n"
        "these values are stored in the `inp_key` enum, defined in\n"
        "the [inp header](/inc/inp.h).\n\n"
    );

    CMD_ADD(basebar,
        Set the window base-bar string,
        "Sets the string displayed in the window bar. This by default shows\n"
        "the name of the current buffer, the current cursor position and\n"
        "the current mode. Escape sequences prefixed with '%' are used here\n"
        "to set the content of the bar:\n\n"

        "| Sequence | Produces |\n"
        "| -------- | -------- |\n"
        "| %L, %C | The line and column of the primary cursor.|\n"
        "| %l, %c | The line and column of the secondary cursor.|\n"
        "| %w, %h | The width and height of the window.|\n"
        "| %x, %y | The x and y position of the window.|\n"
        "| %b     | The number of lines in the current buffer.|\n"
        "| %p     | The percentage of lines in the current buffer below the top"
        "of the page. |\n"
        "| %n     | The name of the current buffer.|\n"
        "| %m     | The name of the current mode.|\n"
        "| %f     | The full filename of the current file.|\n"
        "| %X     | Blank, this is useful for terminating colours.|\n"
        "| %%     | A literal percentage sign.|\n\n"

        "Colours can also be specified, in the format `%fg,bg,attrs`. The\n"
        "background and attributes are optional however, making `%fg` and\n"
        "`%fg,bg` valid sequences. All three options are specified by numbers\n"
        "that correspond to the enums defined in [inc/col.h](/inc/col.h).\n\n"

        "For the background and foreground, the following are the the colour\n"
        "codes:\n\n"

        "| Code | Colour |\n"
        "| ---- | ------ |\n"
        "| 0    | Black  |\n"
        "| 1    | Red    |\n"
        "| 2    | Green  |\n"
        "| 3    | Yellow |\n"
        "| 4    | Blue   |\n"
        "| 5    | Magenta|\n"
        "| 6    | Cyan   |\n"
        "| 7    | White  |\n"
        "| 16   | None   |\n\n"

        "Adding 8 to one of the first 8 colour codes will produce a brighter\n"
        "version of that colour. For example, Black (0) plus 8 produces\n"
        "grey.\n\n"

        "For the attributes, the following values may be summed:\n\n"

        "| Code | Effect     |\n"
        "| ---- | ------     |\n"
        "| 1    | Bold       |\n"
        "| 2    | Underlined |\n"
        "| 4    | Inverted   |\n"
        "| 8    | Blinking   |\n\n"

        "The default basebar is ' **%n** **%8**\xe2\x94\x82**%0**\n"
        "**%L**\xc2\xb7**%C** **%8**\xe2\x94\x82**%0** **%m**'.\n"
    );
}
