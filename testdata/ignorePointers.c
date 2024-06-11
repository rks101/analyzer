
int main()
{
  int x;
  int *ptr;

  ptr = &x;

  *ptr = 10;

  x = 20;

  x = *ptr;

  return 0;
}

