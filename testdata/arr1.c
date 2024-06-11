
#include <stdio.h>

int main()
{
  int dummy[5];
  int arr[10] = {1, 2, 3, 4, 5 ,6, 7, 8, 9, 0};
  int i, sum;

  sum = 0;

  arr[9] = 10;

  //for (i = 0; i < 10; i++)
  //  sum = sum + arr[i];

  printf(" sum = %d \n", sum);

  return 0;
}

