#include <stdio.h>

int main()
{
  enum state { OFF, ON, NONE };

  int x = 2;
  int error;

  enum state b = ON;

  b = b << x;

  if (b)
  {
    error = 1;
  }

  printf("OFF = %d ON = %d b = %d\n", OFF, ON, b);
  return 0;
}

