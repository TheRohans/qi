#ifndef QI_UNICODE_H
#define QI_UNICODE_H

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

typedef uint32_t rune;

#define L(c) (rune)c

int codepoint_len(const rune cp);

void to_utf8(char *buff, const rune cp);

int utf8_len(const char ch);

rune to_rune(const char chr[4]);

int str_to_utf8(const char *src, rune *dest, int src_size);
 
#endif // end guard

