#include <stdio.h>

int main()
{
  int x = 5, y = 4, z = 0;

  printf("z = %d\n\n", z);

  z = x & y;
  printf("z = %d\n\n", z);

  z = x | y;
  printf("z = %d\n\n", z);

  z = x ^ y;
  printf("z = %d\n\n", z);


  printf("x = %d\n\n", x);

  x &= y;
  printf("x = %d\n\n", x);

  x |= y;
  printf("x = %d\n\n", x);

  x ^= y;
  printf("x = %d\n\n", x);

  return 0;
}

