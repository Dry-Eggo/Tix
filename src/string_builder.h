#pragma once
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SB_INI 255
#define sbtcstr(sb) (sb)->data;
#define sbapp(sb, ...) SB_append(sb, __VA_ARGS__);

typedef struct {
  char *data;
  int len;
  int cap;
} StringBuilder;

static void SB_init(StringBuilder **sb) {
  *sb = (StringBuilder *)malloc(sizeof(StringBuilder));
  (*sb)->data = (char *)malloc(SB_INI);
  (*sb)->len = 0;
  (*sb)->cap = SB_INI;
  (*sb)->data[0] = '\0';
}

static void SB_append(StringBuilder *sb, const char *cstr, ...) {
  va_list args;
  va_start(args, cstr);
  int needed = vsnprintf(NULL, 0, cstr, args);
  va_end(args);
  int tneeded = sb->len + needed;
  if (tneeded + 1 > sb->cap) {
    while (sb->cap < tneeded + 1) {
      sb->cap *= 2;
    }
    char *newbuf = (char *)realloc(sb->data, sb->cap);
    sb->data = newbuf;
  }
  va_start(args, cstr);
  int written = vsnprintf(sb->data + sb->len, sb->cap - sb->len, cstr, args);
  sb->len += written;
  sb->data[sb->len] = '\0';
  va_end(args);
}
static void SB_set(StringBuilder *sb, const char *cstr, ...) {
  va_list args;
  va_start(args, cstr);
  int needed = vsnprintf(NULL, 0, cstr, args);
  va_end(args);
  int tneeded = sb->len + needed;
  if (tneeded + 1 > sb->cap) {
    while (sb->cap < tneeded + 1) {
      sb->cap *= 2;
    }
    char *newbuf = (char *)realloc(sb->data, sb->cap);
    sb->data = newbuf;
  }
  va_start(args, cstr);
  int written = vsnprintf(sb->data, sb->cap - sb->len, cstr, args);
  sb->len = needed;
  sb->data[sb->len] = '\0';
  va_end(args);
}
static void SB_free(StringBuilder *s) {
  free(s->data);
  free(s);
  s = NULL;
}
