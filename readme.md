# cfmt

format strings in c. supports custom formatters. passes `va_list` by value, so probably not portable. works on my computer tho

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
  const int n = va_arg(ap, int);
  const int m = va_arg(ap, int);
  // see also: vemitf()
  return emitf(s, "n=%d, m=%d, n+m=%d", n, m, n + m);
}

int main() {
  add_formatter("custom", fmt_custom);
  print("result: {custom}\n", 67, 727);  // prints: result: n=67, m=727, n+m=794
}
```

## escaped braces

```c
print("braces: {{ and }}\n");  // prints: braces: { and }
```

## shoutouts
- [tsoding](https://www.youtube.com/@Tsoding): has been showing up on my youtube feed + where i learned about stb from
- [leaky abstractions](https://github.com/LeakyAbstractions): has nice aesthetic
