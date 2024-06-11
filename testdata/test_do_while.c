
#include <stdio.h>

int main()
{
	int i = 5;

	do
	{
		i = i - 1;
		//i = i >> 1;

		printf("i = %d\n", i);

		//if (i == 0)
		//	return -1;
	}
	while (i > 0);

	return 0;
}

