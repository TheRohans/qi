#ifndef QI_PLUG_DIRED_H
#define QI_PLUG_DIRED_H

void dired_free(EditState *s);

void build_dired_list(EditState *s, const char *path);

/* open dired window on the left. The directory of the current file is
   used */
void do_dired(EditState *s);

int dired_init(void);

#endif

