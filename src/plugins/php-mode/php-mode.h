#ifndef QI_PLUG_PHP_H
#define QI_PLUG_PHP_H

void php_colorize_line(unsigned int *buf, int len, 
                     int *colorize_state_ptr, int state_only);

int php_mode_init(EditState *s, ModeSavedData *saved_data);
int php_init(void);

#endif

