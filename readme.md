# cfmt

format strings in c. supports custom formatters. passes `va_list` by value, so probably not portable. works on my computer tho

`str_view` and `str_builder` included gratis

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

## str_view

non-owning string reference. just a pointer and a length, no heap allocation

```c
str_view a = sv_create("hello", 5);
str_view b = sv_create("world", 5);

sv_cmp(a, b);          // compare two views
sv_cmp_s(a, "hello");  // compare view to c string
```

## str_builder

growable string buffer. allocates on the heap

```c
str_builder sb = sb_create(16);  // start with 16 bytes
sb_append(&sb, "hello ");
sb_append(&sb, "%s %d", "world", 42);
// sb.buf now contains "hello world 42"
sb_free(&sb);
```

## shoutouts
- [tsoding](https://www.youtube.com/@Tsoding): has been showing up on my youtube feed + where i learned about stb from
- [leaky abstractions](https://github.com/LeakyAbstractions): has nice aesthetic
