#include <stdio.h>
int main()
  {
  int z =2, x = 1, y =4, a=1,d;
  //x = ++y*2;
  //z = x++ -y;
  x = y << (y+z);
  x = y << 2;
  x = z = y << 2;
  //--a;
  //++a;  
  //--a;
  //a++;
  a <<= x += y++ + z;
  //d = a++ + a++ + a++ + a;
  //--a;
  //a--;
  //a--;
  printf("x =%d, y =%d, z =%d, a =%d, d =%d",x,y,z,a,d);
  //x = y-2;
  return 0;
  }
