#ifndef QI_UNICODE_H
#define QI_UNICODE_H

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

typedef uint32_t rune;

typedef struct {
	char mask;       /* char data will be bitwise AND with this */
	char lead;       /* start bytes of current char in utf-8 encoded character */
	uint32_t beg;    /* beginning of codepoint range */
	uint32_t end;    /* end of codepoint range */
	int bits_stored; /* the number of bits from the codepoint that fits in char */
}utf_t;

/* len of associated utf-8 char (lengths are in bytes) */
int codepoint_len(const rune cp);
char *to_utf8(const rune cp);
rune to_rune(const char chr[4]);
 
#endif // end guard

