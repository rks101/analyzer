int main()
{
  int a,b,c;
  for(int i=0;i<10;i++)
  {
    for(int j  = 0; j<10; j++)
    {
      for(int k = 0; k<10;k++)
      {
        a+=b+c;
        if(a<0)
        {
          break;
        }
      }
    }
  }
  return a;
}
