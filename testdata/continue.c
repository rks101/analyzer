
#include <stdio.h>
#include <math.h>

int main()
{
	int count, negative;
	double num, root;

	printf("Enter 999 to stop\n");

	count=0;
	negative=0;

	while(count<=10)
	{
		printf("Count is %d\n", count);
		printf("Enter a number\n");
		scanf("%lf", &num);

		if(num==999)
			break;

		if(num<0)
		{
			printf("Number is negative\n");
			negative++;
			continue;
		}

		//root=sqrt(num);
		printf("Number =    %lf\n Root =   %lf\n\n", num,root);
		count++;
	}

	printf("Number of negative items seen %d\n", negative);

	return 0;
}
