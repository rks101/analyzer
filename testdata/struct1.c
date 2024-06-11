
#include <stdio.h>

int main()
{
  struct student
  {
    int roll_number;
    float cpi;
  };

  struct student s1;

  s1.roll_number = 113050099;
  s1.cpi = 7.5;

  printf("s1.roll_number = %d\n", s1.roll_number);
  printf("s1.cpi = %f\n", s1.cpi);

  return 0;
}

