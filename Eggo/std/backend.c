#include "unistd.h"
#include <stdio.h>
#include <string.h>

int tixc_print(const char *msg) {
  write(1, msg, strlen(msg));
  return 0;
}

int tixc_mov_str_buf(char *src, char *dst) {
  strcpy(dst, (void *)src);
  return 0;
}

int tixc_print_buf(char *msg) {
  printf("%s", msg);
  fflush(stdout);
  return 0;
}

int tixc_print_int(const int i) {
  printf("%d", i);
  fflush(stdout);
  return 0;
}

void tixc_strjoin(char *dst, const char *src, char *str) {
  strcpy(dst, src);
  strcat(dst, str);
}

void tixc_read(char *dst) {
  char buffer[1024];
  fgets(buffer, sizeof(buffer), stdin);
  strcpy(dst, buffer);
}
