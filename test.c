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

  // str_view tests
  str_view a = sv_create("hello", 5);
  str_view b = sv_create("hello", 5);
  str_view c = sv_create("world", 5);
  str_view d = sv_create("hi", 2);

  assert(!sv_cmp(a, b));      // equal strings
  assert(sv_cmp(a, c) < 0);   // "hello" < "world"
  assert(sv_cmp(c, a) > 0);   // "world" > "hello"
  assert(sv_cmp(a, d) < 0);   // "hello" < "hi"
  assert(!sv_cmp_s(a, "hello"));
  assert(sv_cmp_s(a, "world") < 0);

  // str_builder tests
  str_builder sb = sb_create(16);
  assert(sb.buf != NULL);
  assert(sb.capacity == 16);
  assert(sb.size == 0);

  sb_append(&sb, "hello ");
  sb_append(&sb, "%s %d", "world", 42);
  assert(sb.size == 14);

  assert(sb_create(0).buf != NULL); // empty builder

  printf("ok\n");
  sb_free(&sb);

  return 0;
}
