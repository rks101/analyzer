#include <stdio.h>

int main()
{
  unsigned long int a = 1;
  int x = 10;
  int y = 13;
  double d = 1.1;
/*
  a = 1;

  x += 20;
  a += 20;


  a = 1 + 2;

  x += 20;
  a += 20;

  x -= 20;
  a -= 20;

  a = 2;

  x *= 20;
  a *= 20;

  x /= 20;
  a /= 20;

  a = 20 * 20;

  x %= 20;
  a %= 20;

  x += a - 3;

  x += -8 + y;

  y = 2;

  x *= y++;

  a = 10;

  x += a *= 4;

  x *= y * -3;
*/
  //x += y += x + 1;

  //x *= y++ * -3;

  // due to ICE
  x += -4 * a;
  x = a * -4;
  x = -4 * a;
  // due to PE
  x += a += (10 +20);
  x = (10+20);

  //printf("\nx = %d, a = %ld\n", x, a);

  //due to ICE
  a += (x+d);
  d += (a+x);

  return 0;
}
