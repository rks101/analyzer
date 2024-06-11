int main()
{
  int a,b,c;
  while(a>0)
  {
    while(c > 0)
    {
      if(c<0)
        b= b-1; 
      else
        break;
      c = c-1;
    }
    a = a-1;
  }
  return 0;
}
