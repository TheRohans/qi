#ifndef QI_PLUG_CSS_H
#define QI_PLUG_CSS_H

int css_init(void);

void css_colorize_line(unsigned int *buf, int len, int *colorize_state_ptr, int state_only);

#endif

