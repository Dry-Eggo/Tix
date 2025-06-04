#ifndef TIX_INTERNALS
#define TIX_INTERNALS

#include "lexer.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TIX_LOG(stream, label, fmt, ...)                                       \
  _tix_log((FILE *)stream, #label, (const char *)(fmt), ##__VA_ARGS__);

#define NEW(T) (T *)malloc(sizeof(T))

static char **stream;
static int stream_max;

static inline void _tix_log(FILE *stream, const char *label, const char *fmt,
                            ...) {
  va_list arg;
  va_start(arg, fmt);
  fprintf(stream, "[%s]: ", label);
  vfprintf(stream, fmt, arg);
  fprintf(stream, "\n");
  va_end(arg);
}

[[noreturn]] static inline void tix_error(Span loc, const char *msg,
                                          char **source, char *help) {
  fprintf(stderr, "\033[1;31m[Tix Error]\033[0m: %s\n", msg);
  char *line = source[loc.line - 1];
  int line_len = strlen(line);

  if (loc.start >= line_len || loc.end > line_len || loc.start >= loc.end) {
    TIX_LOG(stderr, ERROR, "Invalid Location");
    exit(1);
  }

  int len = loc.end - loc.start;
  char *slice = (char *)malloc(len + 1);
  if (!slice) {
    TIX_LOG(stderr, ERROR, "Unable to fetch source info");
    exit(1);
  }
  strncpy(slice, line + loc.start, len);
  slice[len] = '\0';
  fprintf(stderr, "   │\n");
  fprintf(stderr, "  %d│ %s\n", loc.line, line);
  fprintf(stderr, "   │");
  for (int i = 0; i <= line_len; ++i) {
    if (i == loc.start + 1) {
      fprintf(stderr, "\033[33m^");
      while (i < loc.end) {
        fprintf(stderr, "^");
        i++;
      }
      if (help) {
        fprintf(stderr, ":\033[33m %s\033[0m\n", help);
      }
    } else {
      fprintf(stderr, " ");
    }
  }
  fprintf(stderr, "\033[0m\n");
  exit(1);
}

static inline int TString_cstr_to_cstr_array(const char *string, char ***buf) {
  *buf = (char **)malloc(1024 * sizeof(char *));
  int line = 0;
  int inx = 0;
  char accumulator[256] = {0};
  while (*string) {
    if (*string == '\n') {
      accumulator[inx] = '\0';
      string++;
      (*buf)[line++] = strdup(accumulator);
      inx = 0;
    } else {
      accumulator[inx++] = *string;
      string++;
    }
  }
  return line;
}

static void tstrcatf(char *dst, const char *fmt, ...) {
  int len = strlen(dst);
  if (!fmt && !dst)
    return;
  va_list args;
  va_start(args, fmt);
  vsprintf(dst + len, fmt, args);
  va_end(args);
}

#endif
