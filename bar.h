#if !defined(BAR_H)
# define BAR_H

void bar_init(void);

void bar_query(char *str, void (*cb)(char *str));

void bar_query_run(void);

void bar_update(void);

void bar_cmd_ins(char *str, size_t n);

void bar_cmd_del(void);

#endif /* BAR_H */
