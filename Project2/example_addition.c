#include <stdio.h>
#include <immintrin.h>

int arr[8] = {32, 134, 543, 2234, 6423, 2, 8, 42};
int brr[8] = {8, 42, 6423, 2, 32, 2234, 134, 543};
int *crr;

int main() {
	int x, i;

	__m256i a = *((__m256i *)arr);
	__m256i b = *((__m256i *)brr);

	__m256i c = _mm256_add_epi32(a, b);

	crr = (int  *) &c;

	for(x=0; x<8; x++) {
		printf("%d + %d = %d\n", arr[x], brr[x], crr[x]);
	}

	int *aptr = (int *) &a;
	int *bptr = (int *) &b;
	int *cptr = (int *) &c;

	printf("\n\n Dumping __m256i a, b, c:\n");

	for(i=0; i<8; i++)
		printf("a: %d,\t\t b: %d,\t\t c: %d\n", aptr[i], bptr[i], cptr[i]);

	return 0;
}