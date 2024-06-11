#include <stdio.h>

int main()
{
  int x=1, y=2;
  unsigned int ui=100;

  x = ui + 1;
  x = 10 + ui;
  x = ui + y;
  x = y + ui;
  x = ui + -y;
  x = -y + ui;

  y = 1.2;
  y = (2.3);

  float a=11.1, b=12.1;
  double d;
  d=200;

  a = d + 1.2;
  a = 11.2 + d;
  a = d + b;
  a = b + d;

  int x1=1, y1=2;
  unsigned int ui1=100;
  float a1=11.1, b1=12.1;
  double d1;
  d1=200;

  x1 = y1 + ui1 + a1 + d1;
  x1 = y1 * (a1 + ui1) + d1;

  printf("\nx = %d a = %f x1 = %d \n", x, a, x1);

  return 0;
}
