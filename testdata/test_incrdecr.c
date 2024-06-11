//#include <stdio.h>
int main()
{
int a=4;
int c=6;
//int b = a++ + -c;
int b = -c + -a++;
int d=2;
int e;
//printf("\na = %d, b =%d, c = %d, d = %d, e = %d\n", a,b,c,d,e);

e = a++ + a;
//d = c+a;       //a=4,b= -, c = 6,d = 10
//++a;           //a=5,b= -, c = 6,d = 10
//c++;           //a=5,b= -, c = 7,d = 10
//a++;
d = a + a++ + a;// + a++;// + a++ + a + a++;
d = a++ + a++ + a++ + a + ++a + --a +a +a + --a +a-- - a--  + a++ + a++ + a++ + a + ++a + --a +a +a + --a +a-- - a-- + a++ + a++ + a++ + a + ++a + --a +a +a + --a +a-- - a-- - a++ + a++ + a++ + a + ++a + --a +a +a + --a +a-- - a-- - a++ + a++ + a++ + a + ++a + --a +a +a + --a +a-- - a-- + a++ + a++ + a++ + a + ++a + --a +a +a + --a +a-- - a-- * a++ + a++ + a++ + a + ++a + --a +a +a + --a +a--;// - a-- + a++ + a++ + a++ + a + ++a + --a +a +a + --a +a-- - a-- ;       //a=6,b= -, c = 7,d = 5
/*
a = a + a++;

--a;
++a;
--a;
a++;
--a;
a--;
a--;

*/
/*
a = a + a++;

b = d++ + d++ + d++; //a=6,b=12, c = 8,d = 8
e = d++ + a-- + + c --  - --b + c + ++d + a;
//e = d++ + ++d + d + --d + d--;
*/
//printf("\na = %d, b =%d, c = %d, d = %d, e = %d\n", a,b,c,d,e);
return 0;
}
