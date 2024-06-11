#include <stdio.h>

int main()
{
	int i = 5, j = 10, k = 15;

	while (j++ > i)
	{
		//i = i - 1;
		j += 1;
		k--;

		printf("i = %d\n", i);

		//if (i == 0)
		//	return ;
	}

	return 0;
}

