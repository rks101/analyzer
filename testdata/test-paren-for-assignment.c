
int main()
{
  int x = 11, y = 21, z = 31;

  (x) = ((y));
  x = (-y);
  x = -(y);
  (x) = (-y + -(z));
  x = -( -y);

  x = (+y);
  x = +(y);
  (x) = (+y + +(z));

  return 0;
}

