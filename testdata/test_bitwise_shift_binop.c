#include <stdio.h>

int main()
{
  int x = 5, y = 4, z = 0;

  printf("z = %d\n\n", z);

  z = -1 << y;
  printf("z = %d\n\n", z);

  z = x >> y;
  printf("z = %d\n\n", z);

  printf("x = %d\n\n", x);

  x <<= y;
  //x <<= y = z;
  //x <<= y <<= z;
  printf("x = %d\n\n", x);

  x >>= 3;
  printf("x = %d\n\n", x);

  return 0;
}
