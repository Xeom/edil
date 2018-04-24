#if !defined(FILE_H)
# define FILE_H
# include <stdio.h>
# include "vec.h"

typedef enum
{
    file_cr   = 0x01,
    file_pipe = 0x02,
} file_flags;

typedef struct file_s file;

struct file_s
{
    vec fname;
    vec dirname;
    vec basename;
    FILE *fptr;
    file_flags flags;
};

# include "buf.h"

void file_init(file *f);

void file_init_pipe(file *f, FILE *pipe);

void file_kill(file *f);

char *file_base(file *f);

char *file_name(file *f);

int file_assoc(file *f, vec *chrname);

void file_deassoc(file *f);

int file_associated(file *f);

int file_exists(file *f);

int file_open(file *f, const char *mode);

int file_close(file *f);

void file_kill(file *f);

int file_load(file *f, buf *b);

int file_load_line(file *f, buf *b);

int file_save(file *f, buf *b);

int file_save_line(file *f, buf *b, cur loc);

int file_set_paths(file *f, vec *chrname);

#endif
