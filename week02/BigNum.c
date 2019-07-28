// BigNum.c ... LARGE positive integer values

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BigNum.h"

// Initialise a BigNum to N bytes, all zero
void initBigNum (BigNum *bn, int Nbytes)
{	
	assert(bn != NULL);
	bn->nbytes = Nbytes;
	bn->bytes = malloc(sizeof(Byte)*Nbytes);
	assert(bn->bytes != NULL);
	for(int i = 0; i < Nbytes; i++) {
		bn->bytes[i] = 0;
	}
}

// Add two BigNums and store result in a third BigNum
void addBigNums (BigNum bnA, BigNum bnB, BigNum *res)
{
	Byte *max;
	Byte *min;
	int maxsize;
	int minsize;
	if(bnA.nbytes >= bnB.nbytes) {
		maxsize = bnA.nbytes;
		minsize = bnB.nbytes;
		max = bnA.bytes;
		min = bnB.bytes;
	} else {
		maxsize = bnB.nbytes;
		minsize = bnA.nbytes;
		max = bnB.bytes;
		min = bnA.bytes;
	}
	Byte *bytes = malloc(sizeof(Byte)*(maxsize+1));
	int i = 0;
	int sum;
	int ten = 0;
	while(i < maxsize) {
		if(i >= minsize) {
			sum = max[i] + ten;
		}else{
			sum = max[i] + min[i] + ten;
		}
		if(sum > 9) {
			ten = 1;
			sum = sum - 10;
		}else{
			ten = 0;
			sum = sum;
		}
		bytes[i] = sum;
		i++;
	}

	free(res->bytes);
	if(ten == 1) {
		bytes[maxsize] = 1;
		res->nbytes = maxsize + 1;
	}else{
		bytes[maxsize] = 0;
		res->nbytes = maxsize;
	}
	res->bytes = bytes;
}

// Set the value of a BigNum from a string of digits
// Returns 1 if it *was* a string of digits, 0 otherwise
int scanBigNum (char *s, BigNum *bn)
{
	char *c;
	int count = 0;
	for(c = s; *c != '\0'; c++) {
		if(isdigit(*c)){
			count++;
		}
	}
	if(count == 0) {
		return 0;
	}

	//Save digits
	int i = 0;
	Byte *bytes = malloc(sizeof(Byte)*count);
	for(c = s; *c != '\0'; c++) {
		if(isdigit(*c)) {
			bytes[count - i - 1] = *c - '0';
			i++;
		}
	}
	free(bn->bytes);
	bn->bytes = bytes;
	bn->nbytes = count;

	return 1;
}

// Display a BigNum in decimal format
void showBigNum (BigNum bn)
{
	int i = bn.nbytes-1;
	int check = 0;
	while(i >= 0) {
		if(bn.bytes[i] != 0) {
			check = 1;
		}
		if(check == 0 && i == 0) {
			printf("0");
			break;
		}
		if(check == 1) {
			printf("%d",bn.bytes[i]);
		}
		i--;
	}
}
