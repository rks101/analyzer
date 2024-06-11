#include <stdio.h>

//#define E2S(X) ("X")
//#define concate(a, b)	("a -> b")
//#define E2S(X) ("X")
#define concate(a, b)	("\"a\" -> \"b\"")

typedef enum
{
	TN_IF,
	TN_FOR,
	TN_LEAF,
	TN_NONE,
} tn_t;

int main()
{
	FILE *fp;

	fp = fopen("file99.txt", "w");

	char *str;

	//str = (char *) ( concate(E2S(TN_IF), E2S(TN_LEAF)) );
	str = (char *) ( concate(TN_IF, TN_LEAF) );

	printf("\n%s\n", str);

	fputs(str, fp);

	fclose(fp); 

	return 0;
}
