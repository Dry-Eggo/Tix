

extern long __tsys1(long a1);
extern long __tsys2(long a1, long a2);
extern long __tsys3(long a1, long a2, long a3);
extern long __tsys4(long a1, long a2, long a3, long a4);
extern long __tsys5(long a1, long a2, long a3, long a4, long a5);
extern long __tsys6(long a1, long a2, long a3, long a4, long a5, long a6);

static char buf[1] = {0};
static char titos_buf[12] = {0};
extern int tput_byte(char n) {
  buf[0] = n;
  return __tsys4(1, 1, (long)buf, 1);
}

extern void tputs(const char *fmt) {
  int len = 0;
  while (fmt[len]) {
    char c = fmt[len];
    tput_byte(c);
    ++len;
  }
}

extern char *titos(long n) {
  int i = 0;
  int is_negative = 0;
  if (n == 0) {
    titos_buf[i++] = '0';
    titos_buf[i] = '\0';
    return &titos_buf[i];
  }
  if (n < 0) {
    is_negative = 1;
    n = -n;
  }
  int num = n;
  while (num > 0) {
    titos_buf[i++] = '0' + (num % 10);
    num /= 10;
  }
  if (is_negative == 1) {
    titos_buf[i++] = '-';
  }
  titos_buf[i] = '\0';
  int start = 0, end = i - 1;
  while (start < end) {
    char temp = titos_buf[start];
    titos_buf[start] = titos_buf[end];
    titos_buf[end] = temp;
    start++;
    end--;
  }
  return titos_buf;
}

extern void tput_i(long i) {
  char *buf = titos(i);
  tputs(&buf[0]);
}
