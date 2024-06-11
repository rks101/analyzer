int main()
{
  int a=4,b =3,c = 4, d =5;
  if(a >0)
  {
    while(b>0)
    {
      b = b-1;
      
      if( c > 0)
      {
        c = c-1;
      }
      else
      {
      c = c+1;
      }
    }
  a = a-1;
  }
  else
  {
  a = a+1;
  }
  return 0;
}
