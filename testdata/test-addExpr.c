#include <stdio.h>

int main()
{
  int x = 10, y = 3, z = 1, w;
  double a = 1.1, b = 2.1, c = 3.1, d;

  w = 1 + 1;
  w = z + 10;
  w = 20 + z;
  w = x + y;
  w = x - y;
  w = x * y;
  w = x / y;
  w = x % y;

  d = 1.2 + 3.4;
  d = c + 4.5;
  d = 5.6 + c;
  d = a + b;
  d = a - b;
  d = a * b;
  d = a / b;

  w = x + y + z;
  w = x + y * x;
  w = x + y + z;
  w = x + y - z;

  w = 1 + 2 - x + y * x - x / y * 2;
  printf ("\n w = %d\n", w);

  w = 1 + 2 - x + y * x * x / y * 2;
  printf ("\n w = %d\n", w);

  w = -(x + 1) + y;
  printf ("\n w = %d\n", w);

  w = -(x + 1) + -(y + -1);
  printf ("\n w = %d\n", w);

  return 0;
}
