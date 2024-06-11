
int main()
{
  int u, v;
  int shift;
 
  shift=1;

  u = +1 << 4;
  u = -1 << 4;
  u = u << 1;
  u = u << v;
  u = u << shift;

  v = 1 >> 4;
  v = -1 >> 4;
  v = v >> 1;
  v = u >> v;
  v = u >> shift;

  return 0;
}

