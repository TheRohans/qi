#ifndef QI_PLUG_HTML_H
#define QI_PLUG_HTML_H

void html_colorize_line(unsigned int *buf, int len, int *colorize_state_ptr, int state_only);

int html_mode_probe(ModeProbeData *p1);
    
int html_mode_init(EditState *s, ModeSavedData *saved_data);
    
int html_init(void);

#endif

