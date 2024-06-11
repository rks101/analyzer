
#include <stdio.h>

int main()
{
	int x = 10, y;

	y = (x >= 0) ? ( (x == 0) ? 0 : 1) : (-1) ;

	//y = (x >= 0) ? 1 : (-1) ;

	printf("x = %d, y = %d\n", x, y);

	return 0;
}
