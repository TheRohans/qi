#ifndef QI_PLUG_CONFIG_H
#define QI_PLUG_CONFIG_H

int config_init(void);

void config_colorize_line(unsigned int *buf, int len, int *colorize_state_ptr, int state_only);

#endif
