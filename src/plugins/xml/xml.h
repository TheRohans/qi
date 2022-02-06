#ifndef QI_PLUG_XML_H
#define QI_PLUG_XML_H

void xml_colorize_line(unsigned int *buf, int len, int *colorize_state_ptr, int state_only);

int xml_mode_probe(ModeProbeData *p1);

int xml_mode_init(EditState *s, ModeSavedData *saved_data);

int xml_init(void);

#endif

