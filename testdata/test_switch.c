
int main()
{
	int choice, x=10, y=20, result;

	printf("\t1. Add \n");
	printf("\t2. Sub \n");
	printf("\t3. Mul \n");
	printf("\t4. Div \n");
	printf("\t9. Exit the program \n");
	begin:
	printf("\tEnter the choice: ");
	scanf("%d", &choice);

	switch(choice)
	{
		case 1:
			result = x + y;
			break;
		case 2:
			result = x - y;
			break;
		case 3:
			result = x * y;
			break;
		case 4:
			result = x / y;
			break;
		case 9:
			printf("\nExiting the program\n");
			break;
		default: 
			printf("\n\tNot a valid choice.\n");
			goto begin;
	}

	printf("result = %d \n", result);

	return 0;
}

