#ifndef _GUID_6a76c5cd_3683_4d9e_a7e3_416a59a66fe4_ASSERT_H_
#define _GUID_6a76c5cd_3683_4d9e_a7e3_416a59a66fe4_ASSERT_H_

#include <cstdio>
#include <cstdlib>

static inline void ASSERT(bool pred, const char *message = "",
                          int line = __builtin_LINE(),
                          const char *file = __builtin_FILE(),
                          const char *function = __builtin_FUNCTION()) {
  if (pred) {
    return;
  }
  fprintf(stderr, "\n%s:%d assertion failed in function %s. %s\n", file, line,
          function, message);
  std::abort();
}

#endif /* _GUID_6a76c5cd_3683_4d9e_a7e3_416a59a66fe4_ASSERT_H_ */