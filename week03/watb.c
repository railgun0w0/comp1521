// COMP1521 19t2 ... lab 03: where are the bits?
// watb.c: determine bit-field order

#include <stdio.h>
#include <stdlib.h>

struct _bit_fields {
	unsigned int a : 4;
	unsigned int b : 8;
	unsigned int c : 20;
};

int main (void)
{
	struct _bit_fields x = {0,0,0};
	unsigned int *ptrX = (unsigned int*)&x;
	unsigned int val = 10;
	unsigned int *valPtr = &val;
	unsigned int mask = 1;
	int i = 0;
	while(i < 32) {
		if((*valPtr & mask) > 0) {
			printf("1");
		} else {
			printf("0");
		}
		i++;
		mask = mask << 1;
	}
	printf("\n");
	printf ("%zu\n", sizeof (x));

	return EXIT_SUCCESS;
}
