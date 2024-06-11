int main()
{
  int x = 10, y = 1, z = x + y + 20, w = x + z + -y, u = x + y + w++, v = x + y + -z + w++;

  return 0;
}
