#ifndef CFMT_H
#define CFMT_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// to avoid name collisions, #define CFMT_VERBOSE
#ifdef CFMT_VERBOSE
#define CFMT_DECORATE(name) CFMT_##name
#define cfmt_decorate(name) cfmt_##name
#define sink                cfmt_decorate(sink)
#define formatter           cfmt_decorate(formatter)
#define FORMATTERS          CFMT_DECORATE(FORMATTERS)
#define add_formatter       cfmt_decorate(add_formatter)
#define OK                  CFMT_DECORATE(OK)
#define OVERFLOW            CFMT_DECORATE(OVERFLOW)
#define SINGLE_RBRACE       CFMT_DECORATE(SINGLE_RBRACE)
#define MISSING_RBRACE      CFMT_DECORATE(MISSING_RBRACE)
#define UNKNOWN_SPEC        CFMT_DECORATE(UNKNOWN_SPEC)
#define vemitf              cfmt_decorate(vemitf)
#define emitf               cfmt_decorate(emitf)
#define print               cfmt_decorate(print)
#endif

#define CFMT_SEG_BUF_LEN  (4096)
#define CFMT_SPEC_BUF_LEN (256)

typedef struct {
  void *self;
  void (*emit)(void *self, const char *s);
} sink;

typedef struct formatter formatter;
struct formatter {
  const char *spec;
  void (*fmt)(const sink s, va_list);
  formatter *next;
};

extern formatter FORMATTERS;
formatter *add_formatter(const char *spec, void (*fmt)(const sink s, va_list));

const int OK             =  0;
const int OVERFLOW       = -1;
const int SINGLE_RBRACE  = -2;
const int MISSING_RBRACE = -3;
const int UNKNOWN_SPEC   = -4;

int vemitf(const sink s, const char *fmt, va_list ap);
int emitf(const sink s, const char *fmt, ...);

void print(const char *fmt, ...);

// todo: delete
#define CFMT_IMPL
#ifdef CFMT_IMPL

formatter FORMATTERS = {};

formatter *add_formatter(const char *spec, void (*fmt)(const sink s, va_list)) {
  formatter *f = calloc(1, sizeof(formatter));
  f->spec = spec;
  f->fmt = fmt;
  f->next = FORMATTERS.next;
  return (FORMATTERS.next = f);
}

int vemitf(const sink s, const char *fmt, va_list ap) {
  const char *seg = fmt;
  while (*fmt) {
    const char c = *fmt++;

    // "}}" -> "}"
    if (c == '}') {
      if (*fmt++ != '}') return SINGLE_RBRACE;
      s.emit(s.self, "}");
    }

    else if (c == '{') {
      //  "{{" -> "{"
      if (*fmt == '{') {
        s.emit(s.self, "{");
        fmt++;
      }

      else {
        char buf[CFMT_SEG_BUF_LEN];

        // emit current segment up to right before '{'
        {
          const int seg_len = (fmt - 1) - seg;
          if (seg_len >= sizeof(buf)) return OVERFLOW;
          memcpy(buf, seg, seg_len);
          buf[seg_len] = '\0';
          s.emit(s.self, buf);
        }

        // process interpolation
        {
          // parse format specifier
          char spec_buf[CFMT_SPEC_BUF_LEN];
          const char *spec_start = fmt;
          while (*fmt && *fmt != '}') fmt++;
          if (*fmt != '}') return MISSING_RBRACE;
          const int spec_len = fmt - spec_start;
          if (spec_len >= sizeof(spec_buf)) return OVERFLOW;
          memcpy(spec_buf, spec_start, spec_len);
          spec_buf[spec_len] = '\0';
          fmt++;

          // built-in specifier (could not be bothered to implement)
          if (spec_buf[0] == '%') {
            const int len = vsnprintf(buf, sizeof(buf), spec_buf, ap);
            if (len >= sizeof(buf)) return OVERFLOW;
            s.emit(s.self, buf);
          }

          // custom specifier
          else {
            formatter *f = FORMATTERS.next;
            while (f) {
              if (!strcmp(spec_buf, f->spec)) {
                f->fmt(s, ap);
                break;
              }
              f = f->next;
            }
            if (!f) return UNKNOWN_SPEC;
          }

          seg = fmt;
        }
      }
    }
  }

  // emit last segment
  if (*seg) s.emit(s.self, seg);

  return 0;
}

int emitf(const sink s, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  const int ret = vemitf(s, fmt, ap);
  va_end(ap);
  return ret;
}

void _emit_print(void *self, const char *s) {
  printf("%s", s);
}

void print(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vemitf((sink) { .emit = _emit_print }, fmt, ap);
  va_end(ap);
}

#endif

#ifdef CFMT_VERBOSE
#undef emitf
#undef vemitf
#undef UNKNOWN_SPEC
#undef MISSING_RBRACE
#undef SINGLE_RBRACE
#undef OVERFLOW
#undef OK
#undef add_formatter
#undef FORMATTERS
#undef formatter
#undef sink
#undef cfmt_decorate
#undef CFMT_DECORATE
#endif

#endif
