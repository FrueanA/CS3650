#include <stdio.h>

int main(int argc, char **argv) {
  long l;
  char c;

  scanf("%c %ld", &c, &l);

  printf("%ld %c\n", l, c);

  return 0;
}
