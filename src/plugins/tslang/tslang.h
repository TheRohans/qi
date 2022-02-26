#ifndef QI_PLUG_TSLANG_H
#define QI_PLUG_TSLANG_H

// For other plugins to syntax highlight, for 
// example HTML mode
void ts_colorize_line(unsigned int *buf, int len, int *colorize_state_ptr,
                     int state_only);

int ts_mode_init(EditState *s, ModeSavedData *saved_data);
int ts_init(void);

#endif
