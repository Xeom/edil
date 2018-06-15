#if !defined(INP_H)
# define INP_H
# include "types.h"

extern table inp_keytranslate;

inp_key inp_get_key(unsigned char c);

void inp_key_name(inp_key key, char *str, size_t len);

void inp_empty_pipe(void);

void inp_init(void);

void inp_kill(void);

void inp_wait(void);

int inp_key_cmp(const void *aptr, const void *bptr);

#endif
