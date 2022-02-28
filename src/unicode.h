#ifndef QI_UNICODE_H
#define QI_UNICODE_H

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

typedef uint32_t rune;

#define L(c) (rune)c

/* len of associated utf-8 char (lengths are in bytes) */
int codepoint_len(const rune cp);

void to_utf8(char *buff, const rune cp);

rune to_rune(const char chr[4]);
 
#endif // end guard

