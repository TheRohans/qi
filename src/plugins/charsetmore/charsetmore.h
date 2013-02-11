
void decode_8bit_init(CharsetDecodeState *s);

/* not very fast, but not critical yet */
unsigned char *encode_8bit(QECharset *charset, unsigned char *q, int c);

//static int jis0208_decode(int b1, int b2);

//static int jis0212_decode(int b1, int b2);

//static void decode_euc_jp_init(CharsetDecodeState *s);

//static int decode_euc_jp_func(CharsetDecodeState *s, const unsigned char **pp);

//static unsigned char *encode_euc_jp(QECharset *s, unsigned char *q, int c);

//static void decode_sjis_init(CharsetDecodeState *s);

/* XXX: add state */
//static int decode_sjis_func(CharsetDecodeState *s, const unsigned char **pp);

//static unsigned char *encode_sjis(QECharset *s, unsigned char *q, int c);

int charset_more_init(void);
