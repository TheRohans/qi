#include "../r2_unit.h"
// ###################
#include "../cutils.c"  // needed for pstrcpy
// ###################
#include "../util.c"

#define INPUT_TOLOWER 1
#define INPUT_UNTOUCH 0

#define FOUND 1
#define NOT_FOUND 0

static char *test_strfind_find_nocase() {
  int rtn = strfind("|LICENSE|COPYING|", "LICENSE", INPUT_UNTOUCH);
  r2_assert("find no case failed", rtn == FOUND);
  return 0;
}

static char *test_strfind_notfind_nocase() {
  int rtn = strfind("|LICENSE|COPYING|", "LC", INPUT_UNTOUCH);
  r2_assert("no find, no case failed", rtn == NOT_FOUND);
  return 0;
}

static char *test_strfind_notfind_case() {
  int rtn = strfind("|LICENSE|COPYING|", "license", INPUT_UNTOUCH);
  r2_assert("no find with case failed", rtn == NOT_FOUND);
  return 0;
}

static char *util_test() {
  r2_run_test(test_strfind_find_nocase);
  r2_run_test(test_strfind_notfind_nocase);
  r2_run_test(test_strfind_notfind_case);
  return 0;
}
