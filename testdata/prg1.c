
#define MAX 1024
#define TWICE(x) ((x) * (x))

int a, b;
double d;

int dim = MAX;

struct s1
{
  int si;
  double sd;
};

int main()
{
  int x, y = 2147483647, new = y;
  long int dummy = 2147483648;
  unsigned long long int ulli;
  unsigned long int z = 0;

  float ff = 1.23;
  double d = 3.14;
  long double ld = 2.718281828;

  char c = 'A';
  unsigned ch = 'a';

  x = 10;

  x = MAX;

  //x = TWICE(10);

  ulli = 100;

  //z = 0;

  d = 1.2;

  y = -1;

  c = 'a';

  x = +1;

  x = -y;

  x = +y;

  x = x + 1;

  x = -1 + y;

  d = -1.1;

  d = +1.3;

  ld = d + 0.9;

  y = -(x + 1) + y;

  x = y + -(x + 1);

  x = y + -1;

  ulli = 1 + 1;

  ulli = x + 20;

  ulli = x + -20;

  return 0;
}

