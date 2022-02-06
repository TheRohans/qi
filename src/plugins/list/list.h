#ifndef QI_PLUG_LIST_H
#define QI_PLUG_LIST_H

/* get current position (index) in list */
int list_get_pos(EditState *s);

/* get current offset of the line in list */
int list_get_offset(EditState *s);

void list_toggle_selection(EditState *s);

int list_init(void);

#endif

