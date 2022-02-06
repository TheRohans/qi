#ifndef QI_PLUG_CHARSETMORE_H
#define QI_PLUG_CHARSETMORE_H

void decode_8bit_init(CharsetDecodeState *s);

/* not very fast, but not critical yet */
unsigned char *encode_8bit(QECharset *charset, unsigned char *q, int c);

int charset_more_init(void);

#endif
