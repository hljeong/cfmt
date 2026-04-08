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
#define vprint              cfmt_decorate(vprint)
#define fprint              cfmt_decorate(fprint)
#define vfprint             cfmt_decorate(vfprint)
#define snprint             cfmt_decorate(snprint)
#define vsnprint            cfmt_decorate(vsnprint)
#endif


#ifndef CFMT_H
#define CFMT_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CFMT_SEG_BUF_LEN  (4096)
#define CFMT_SPEC_BUF_LEN (256)

typedef struct {
  void *self;
  int (*emit)(void *self, const char *s);
} sink;

typedef struct formatter formatter;
struct formatter {
  const char *spec;
  int (*fmt)(const sink s, va_list);
  formatter *next;
};

extern formatter FORMATTERS;
formatter *add_formatter(const char *spec, int (*fmt)(const sink s, va_list));

const int OK             =  0;
const int OVERFLOW       = -727;
const int SINGLE_RBRACE  = -728;
const int MISSING_RBRACE = -729;
const int UNKNOWN_SPEC   = -730;

int vemitf(const sink s, const char *fmt, va_list ap);
int emitf (const sink s, const char *fmt, ...);

int    print(const char *fmt, ...);
int   vprint(const char *fmt, va_list ap);
int   fprint(FILE *file, const char *fmt, ...);
int  vfprint(FILE *file, const char *fmt, va_list ap);
int  snprint(char *buf, const size_t size, const char *fmt, ...);
int vsnprint(char *buf, const size_t size, const char *fmt, va_list ap);

#endif

// instantiate the implementation with #define CFMT_IMPL
#define CFMT_IMPL
#ifdef CFMT_IMPL

formatter FORMATTERS = {};

formatter *add_formatter(const char *spec, int (*fmt)(const sink s, va_list)) {
  formatter *f = calloc(1, sizeof(formatter));
  f->spec = spec;
  f->fmt = fmt;
  f->next = FORMATTERS.next;
  return (FORMATTERS.next = f);
}

int vemitf(const sink s, const char *fmt, va_list ap) {
  char buf[CFMT_SEG_BUF_LEN];
  int total_len = 0;

  const char *seg = fmt;
  while (*fmt) {
    const char c = *fmt++;
    if (c != '{' && c != '}') continue;

    // emit current segment up to right before `c`
    {
      char seg_fmt[CFMT_SEG_BUF_LEN];
      const int seg_len = (fmt - 1) - seg;
      if (seg_len) {
        if (seg_len >= sizeof(seg_fmt)) return OVERFLOW;
        memcpy(seg_fmt, seg, seg_len);
        seg_fmt[seg_len] = '\0';

        const int len = vsnprintf(buf, sizeof(buf), seg_fmt, ap);
        if (len >= sizeof(buf)) return OVERFLOW;
        const int ret = s.emit(s.self, buf);
        if (ret < 0) return ret;
        total_len += ret;
      }
    }

    if (c == '}') {
      // "}}" -> "}"
      if (*fmt++ != '}') return SINGLE_RBRACE;
      const int ret = s.emit(s.self, "}");
      if (ret < 0) return ret;
      total_len += ret;
      seg = fmt;
    }

    // "{{" -> "{"
    else if (*fmt == '{') {
      fmt++;
      const int ret = s.emit(s.self, "{");
      if (ret < 0) return ret;
      total_len += ret;
      seg = fmt;
    }

    // custom formatter
    else {
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

      // dispatch formatter
      formatter *f = FORMATTERS.next;
      while (f) {
        if (!strcmp(spec_buf, f->spec)) {
          const int ret = f->fmt(s, ap);
          if (ret < 0) return ret;
          total_len += ret;
          break;
        }
        f = f->next;
      }
      if (!f) return UNKNOWN_SPEC;

      seg = fmt;
    }
  }

  // emit last segment
  if (*seg) {
    const int len = vsnprintf(buf, sizeof(buf), seg, ap);
    if (len >= sizeof(buf)) return OVERFLOW;
    const int ret = s.emit(s.self, buf);
    if (ret < 0) return ret;
    total_len += ret;
  }

  return total_len;
}

int emitf(const sink s, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  const int ret = vemitf(s, fmt, ap);
  va_end(ap);
  return ret;
}

typedef struct { FILE *file; } _cfmt_fprint_sink;
typedef struct { char *buf; size_t n; } _cfmt_snprint_sink;

static int _cfmt_print(void *self, const char *s) {
  return printf("%s", s);
}

static int _cfmt_fprint(void *self, const char *s) {
  return fprintf(((_cfmt_fprint_sink *) self)->file, "%s", s);
}

static int _cfmt_snprint(void *self, const char *s) {
  _cfmt_snprint_sink *sink = self;
  const int ret = snprintf(sink->buf, sink->n, "%s", s);
  if (ret < 0) return ret;
  const int advance = ret >= sink->n ? sink->n : ret;
  sink->buf += advance;
  sink->n -= advance;
  return ret;
}

int vprint(const char *fmt, va_list ap) {
  return vemitf((sink) { .emit = _cfmt_print }, fmt, ap);
}

int print(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  const int ret = vprint(fmt, ap);
  va_end(ap);
  return ret;
}

int vfprint(FILE *file, const char *fmt, va_list ap) {
  _cfmt_fprint_sink s = { .file = file };
  return vemitf((sink) { .self = &s, .emit = _cfmt_fprint }, fmt, ap);
}

int fprint(FILE *file, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  const int ret = vfprint(file, fmt, ap);
  va_end(ap);
  return ret;
}

int vsnprint(char *buf, const size_t n, const char *fmt, va_list ap) {
  _cfmt_snprint_sink s = { .buf = buf, .n = n };
  return vemitf((sink) { .self = &s, .emit = _cfmt_snprint }, fmt, ap);
}

int snprint(char *buf, const size_t n, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  const int ret = vsnprintf(buf, n, fmt, ap);
  va_end(ap);
  return ret;
}

#undef CFMT_IMPL

#endif


#undef vsnprint
#undef snprint
#undef vfprint
#undef fprint
#undef vprint
#undef print
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
