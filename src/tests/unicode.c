#include "../r2_unit.h"
#include "../unicode.c"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static char *test_to_utf8()
{
    char *utf8 = to_utf8(0x1d11e);
    r2_assert("to utf8 failed", (strcmp(utf8, "𝄞") == 0));
    return 0;
}

static char *test_to_rune()
{
    char *utf8 = to_utf8(0x1d11e);
    rune codepoint = to_rune(utf8);
    r2_assert("to rune failed", (codepoint == 0x1d11e));
    return 0;
}

static char *test_codepoint_len()
{
    int len = codepoint_len(0x1d11e);
    r2_assert("codepoint len failed", (len == 4));
    return 0;
}

static char *unicode_test()
{
    r2_run_test(test_to_utf8);
    r2_run_test(test_to_rune);
    r2_run_test(test_codepoint_len);
    return 0;
}
