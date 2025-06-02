

extern long __tsys1(long a1);
extern long __tsys2(long a1, long a2);
extern long __tsys3(long a1, long a2, long a3);
extern long __tsys4(long a1, long a2, long a3, long a4);
extern long __tsys5(long a1, long a2, long a3, long a4, long a5);
extern long __tsys6(long a1, long a2, long a3, long a4, long a5, long a6);

static char buf[1] = {0};
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

