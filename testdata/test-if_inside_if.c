int main()
{
  int x = 10;

  if (x >= 0)
  {
    x = 100;

    if (x == 0)
      x = 0;
  }
  else
  {
    x = -1;
  }

  return 0;
}
