/**
 * Modified from the example found here:
 *  https://rosettacode.org/wiki/UTF-8_encode_and_decode#C
 * Content is available under GNU Free Documentation License 1.2
 */
#include "unicode.h"

utf_t * utf[] = {
	/*             mask        lead        beg      end       bits */
	[0] = &(utf_t){0b00111111, 0b10000000, 0,       0,        6    },
	[1] = &(utf_t){0b01111111, 0b00000000, 0000,    0177,     7    },
	[2] = &(utf_t){0b00011111, 0b11000000, 0200,    03777,    5    },
	[3] = &(utf_t){0b00001111, 0b11100000, 04000,   0177777,  4    },
	[4] = &(utf_t){0b00000111, 0b11110000, 0200000, 04177777, 3    },
	      &(utf_t){0},
};

/**
 * Get the number of bytes contained within this rune
 * see table above
 */
int codepoint_len(const rune cp)
{
	int len = 0;
	for(utf_t **u = utf; *u; ++u) {
		if((cp >= (*u)->beg) && (cp <= (*u)->end)) {
			break;
		}
		++len;
	}
	if(len > 4) /* Out of bounds */
		exit(1);
 
	return len;
}
 
/* len of utf-8 encoded char (lengths are in bytes) */
int utf8_len(const char ch)
{
	int len = 0;
	for(utf_t **u = utf; *u; ++u) {
		if((ch & ~(*u)->mask) == (*u)->lead) {
			break;
		}
		++len;
	}
	if(len > 4) { /* Malformed leading byte */
		exit(1);
	}
	return len;
}

/**
 * Take a rune and turn it into an array of chars
 * that can be used as a %s kind of output
 */
char *to_utf8(const rune cp)
{
	static char ret[5];
	const int bytes = codepoint_len(cp);
 
	int shift = utf[0]->bits_stored * (bytes - 1);
	ret[0] = (cp >> shift & utf[bytes]->mask) | utf[bytes]->lead;
	shift -= utf[0]->bits_stored;
	for(int i = 1; i < bytes; ++i) {
		ret[i] = (cp >> shift & utf[0]->mask) | utf[0]->lead;
		shift -= utf[0]->bits_stored;
	}
	ret[bytes] = '\0';
	return ret;
}

/**
 * Given a set of chars (max 4) turn that into a rune. Meaning,
 * look at the first bits of the chars and figure out if they are
 * continuation chars or starter chars and make a rune out of it
 * (if valid).
 */
rune to_rune(const char chr[4])
{
	int bytes = utf8_len(*chr);
	int shift = utf[0]->bits_stored * (bytes - 1);
	rune codep = (*chr++ & utf[bytes]->mask) << shift;
 
	for(int i = 1; i < bytes; ++i, ++chr) {
		shift -= utf[0]->bits_stored;
		codep |= ((char)*chr & utf[0]->mask) << shift;
	}
 
	return codep;
}

/*
int main(void)
{
	const uint32_t *in, input[] = {0x0041, 0x00f6, 0x0416, 0x20ac, 0x1d11e, 0x0};
 
	printf("Character  Unicode  UTF-8 encoding (hex)\n");
	printf("----------------------------------------\n");
 
	char *utf8;
	rune codepoint;
	for(in = input; *in; ++in) {
		utf8 = to_utf8(*in);
		codepoint = to_rune(utf8);
		printf("%s          U+%-7.4x", utf8, codepoint);
 
		for(int i = 0; utf8[i] && i < 4; ++i) {
			printf("%hhx ", utf8[i]);
		}
		printf("\n");
	}
	return 0;
}
*/


