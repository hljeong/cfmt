# cfmt

format strings in c. supports custom formatters. passes `va_list` by value, so probably not portable. works on my computer though

## usage

single-header technique inspired by [stb](https://github.com/nothings/stb)

```c
#define CFMT_IMPL
#include "cfmt.h"

int main() {
  // drop-in replacement for the standard ones
  // except sprintf() and vsprintf() which are
  // worthless pieces of trash
  print("hello %s\n", "world");
}
```

## custom formatters

```c
static int fmt_custom(const sink s, va_list ap) {
  int n = va_arg(ap, int);
  char buf[32];
  snprintf(buf, sizeof(buf), "value=%d", n);
  return s.emit(s.self, buf);
}

int main() {
  add_formatter("custom", fmt_custom);
  print("result: {custom}", 42);  // prints: result: value=42
}
```

## escaped braces

```c
print("braces: {{ and }}");  // prints: braces: { and }
```

## verbose mode

to avoid name collisions, define `CFMT_VERBOSE` before including:

```c
#define CFMT_VERBOSE
#define CFMT_IMPL
#include "cfmt.h"
```

this prefixes all symbols with `cfmt_` (e.g., `cfmt_print`, `cfmt_snprint`)

## shoutouts

- [leaky abstraction](https://github.com/LeakyAbstractions): has nice aesthetic
