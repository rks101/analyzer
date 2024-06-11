
int main()
{
  int u, v;
  int shift;
 
  shift=1;

  u <<= 1;
  u <<= shift;

  v >>= 1;
  u >>= shift;

  return 0;
}

