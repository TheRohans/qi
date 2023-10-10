#ifndef QI_PLUG_SQLLANG_H
#define QI_PLUG_SQLLANG_H

void sql_colorize_line(unsigned int *buf, int len, int *colorize_state_ptr,
                     int state_only);
int sql_mode_init(EditState *s, ModeSavedData *saved_data);
int sql_init(void);

#endif
