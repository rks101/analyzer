
#include <stdio.h>

int main()
{
	int x, y, z = 20, w, u;

	x = y = 10;

	x = y = z;

	//x = y = z = w = u;
        x = y = z = w = x + y;

        printf("x = %d y = %d z = %d w = %d\n", x, y, z, w);

        x = y = z = w = -z;

        printf("x = %d y = %d z = %d w = %d\n", x, y, z, w);

        x = y = z = w = 2*x + y + -z;
        //x = y = z = w = 2*x + y + +z;

        printf("x = %d y = %d z = %d w = %d\n", x, y, z, w);

	return 0;
}
