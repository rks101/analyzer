
#include <stdio.h>

int main()
{
  int a = 4;
  int c = 6;
  int b = -c + a++;
  int d, e;
  a = a++ + a;
  d = a + a++ + a;
  b = d++ + d++ + d++;
  printf("\na = %d, b = %d, c = %d, d = %d, e = %d\n", a, b, c, d, e);
  return 0;
}
