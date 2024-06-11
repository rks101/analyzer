//#include <stdio.h>

int main()
{
	int a,b,max;

	a = 10;
	b = 20;

	if (a>b)
	{
		max = a;
		//a++;
	}
	else
	{
		max = b;
		//a--;
	}

	printf("a=%d, b=%d, max= %d\n",a,b,max);

	return 0;
}
