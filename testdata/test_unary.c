
#include <stdio.h>

int main()
{
  int x = 10, y;
  double a = 1.2, b;

  x = -1;
  a = -1.1;
  y = -x;

  y = -1 + x;
  y = x + -1;
  x = x + -1;
  y = -x + -y;
  y = -(x + 10) + y;
  y = y + -(x + 10) + y;

  x = +1;
  a = +1.1;
  y = +x;

  y = +1 + x;
  y = x + +1;
  x = x + +1;
  y = +x + +y;
  y = +(x + 10) + y;
  y = y + +(x + 10) + y;


  return 0;
}
