#include <sys/types.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>

#include "bind.h"
#include "file.h"
#include "win.h"
#include "vec.h"
#include "cmd.h"
#include "ui.h"

#include "cmd/conf.h"

static char *conf_default_files[] =
{
    ".edil.conf",
    ".edil",
    ".config/edil",
    ".config/edil.conf"
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
#include <stdio.h>

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

void cmd_conf_init(void)
{
    CMD_ADD(conffile, Load a config file, "");
    CMD_ADD(remap, Remap a key, "");
    CMD_ADD(unmap, Unmap a key, "");
    CMD_ADD(translate, Translate a keypress, "");
}