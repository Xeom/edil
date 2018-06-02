#include <sys/types.h>
#include <string.h>
#include <pwd.h>

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

        conf_run_file_name(vec_first(&name), w);
    }
}

void conf_cmd_run_file(vec *rtn, vec *args, win *w)
{
    size_t ind, len;

    len = vec_len(args);
    for (ind = 1; ind < len; ++ind)
    {
        vec *fname;
        file f;

        fname = vec_get(args, ind);

        file_init(&f);
        file_assoc(&f, fname);

        if (!file_exists(&f))
            chr_format(rtn, "err: file '%s' does not exist ", file_name(&f));
        else
        {
            file_open(&f, "r");
            conf_run_file(&f, w);
        }

        file_kill(&f);
    }
}
