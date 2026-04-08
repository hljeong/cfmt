#define CFMT_IMPL
#include "cfmt.h"

#include <assert.h>
#include <string.h>

static int test_fmt(const sink s, va_list ap) {
  (void) ap;
  s.emit(s.self, "custom");
  return 6;
}

int main(void) {
  add_formatter("test", test_fmt);

  char buf[256];

  snprint(buf, sizeof(buf), "hello");
  assert(strcmp(buf, "hello") == 0);

  snprint(buf, sizeof(buf), "num: %d", 42);
  assert(strcmp(buf, "num: 42") == 0);

  snprint(buf, sizeof(buf), "escaped: {{}}");
  assert(strcmp(buf, "escaped: {}") == 0);

  snprint(buf, sizeof(buf), "result: {test}");
  assert(strcmp(buf, "result: custom") == 0);

  printf("ok\n");

  return 0;
}
