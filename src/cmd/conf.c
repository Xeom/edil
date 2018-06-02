#include "file.h"
#include "win.h"
#include "vec.h"
#include "cmd.h"
#include "ui.h"

#include "cmd/conf.h"

void conf_run_file(file *f, win *w)
{
    vec line;
    vec_init(&line, sizeof(chr));

    while (!file_ended(f))
    {
        file_read_line(f, &line);
        ui_cmd_cb(w, &line);
    }

    vec_kill(&line);
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
