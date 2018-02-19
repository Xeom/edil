#include <string.h>

#include "vec.h"
#include "bar.h"
#include "cli.h"

#include "files.h"

void files_load_query(text_buf *b)
{
    bar_query("Load: ", files_load_confirm);
}

void files_load_confirm(char *str)
{
    FILE *f;

    if (strlen(str) == 0) return;

    f = fopen(str, "r");

    if (f) files_load(text_cur_buf, f);

    cli_lines_after(text_cur_buf, 0);

    fclose(f);
}    

void files_load(text_buf *b, FILE *f)
{
    int chr;
    size_t ln;
    vec *line;
    text_cmd cmd;

    ln = text_buf_len(b);

    while (ln--)
    {
        cmd.type = text_cmd_del_line;
        cmd.args.del_line.ln = ln;

        text_buf_cmd(b, &cmd);
    }

    line = &(cmd.args.ins.chrs);
    vec_init(line, sizeof(text_char));
    cmd.type = text_cmd_ins;
    cmd.args.ins.cn = 0;
 
    ln = 1;
    while ((chr = getc(f)) != EOF)
    {
        if (chr != '\n')
        {
            text_char chrval;
            chrval.utf8[0] = chr;
            chrval.utf8[1] = '\0';
            chrval.fg      = text_col_none;
            vec_ins(line, vec_len(line), 1, &chrval);
        }
        else
        {
            text_cmd lcmd;

            text_buf_cmd(b, &cmd);
            vec_del(line, 0, vec_len(line));
            lcmd.type = text_cmd_ins_line;
            lcmd.args.ins_line.ln = ln;
            cmd.args.ins.ln       = ln;

            text_buf_cmd(b, &lcmd);
        }
    }

    text_buf_cmd(b, &cmd);
}

void files_dump(text_buf *b, FILE *f)
{
}
