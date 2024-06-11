#include <stdio.h>
/*
int foo(int a, int b, int c)
{
  return a+b+c;
}

int bar(int a, int b, int c)
{
  return a+b+c;
}
*/
int main()
{
  int x, y, z;

  printf("Enter x, y, z: ");
  scanf("%d %d %d", &x, &y, &z);
  int w, i;
  do {
    if (x < y) {
      //int w;
      printf("Enter w :");
      scanf("%d", &w);
      //while (foo(z, y, w) > x) {
      //  if (w + x < bar(w, w, x)) {
      while ((z+y+w) > x) {
        if (w + x < (w+w+x)) {
          printf("Enter w ");
          scanf("%d", &w);
          while (w + x < z*y) {
            //w+= x;
          }
        }
        else if (w*w > 256) {
          if ((w + x < z*y) || (y > 45)) {
            //for (int i = 0; i < z*y + x; i++) {
            for (i = 0; i < z*y + x; i++) {
              z++; w--;
            }
          }
          y = y + 10;
        }
      }  
    }
    else {
      while (y*y > z) {
        printf("Enter y, z: ");
        scanf("%d %d", &y, &z);
        y = y + w;
      }
    }

  } while (++x < y*z);

  printf("So what we have now:\nx = %d, y = %d, z =%d, w = %d\n", x, y, z, w);

  return 0;
}

