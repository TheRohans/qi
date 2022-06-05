#ifndef QI_PLUG_LUALANG_H
#define QI_PLUG_LUALANG_H

void lua_colorize_line(unsigned int *buf, int len, int *colorize_state_ptr,
                     int state_only);
int lua_mode_init(EditState *s, ModeSavedData *saved_data);
int lua_init(void);

#endif
