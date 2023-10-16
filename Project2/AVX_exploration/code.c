#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>

#define ALIGNED 1
#define UNALIGNED 0
#define ALLOC_MODE ALIGNED

float *arr;
float *brr;
float *crr;
float *drr;

void initializeArrays(size_t *a, size_t *b, int length) {
	*a = (size_t) malloc(2 * length * sizeof(float));
	*b = *a + (length * sizeof(float));

	float *arr = (float *) *a;
	float *brr = (float *) *b;

	
	printf("0x%lX\t0x%lX\n", (size_t) *a, (size_t) *b);
	printf("0x%lX\t0x%lX\n", (size_t) *a, (size_t) *b);

	for(int i=0; i<length; i++) {
		arr[i] = rand() & 0xF;
		brr[i] = rand() & 0xF;
		// randomly alot sign
		arr[i] *= (((int) (rand() << 1)) >> 31) | 0x1;
		brr[i] *= (((int) (rand() << 1)) >> 31) | 0x1;
	}

	printf("\nExiting initializeArrays()\n\n");
	fflush(0);

	return;
}

float fakeOp(float *a, float *b){
	int x;
	float sum = 0.0;

	for(x=0; x<4; x++) {
		sum += a[x] * b[x];
	}

	return sum;
}

int main() {
	int i;
	__m256 a, b;

	initializeArrays((size_t *) &arr, (size_t *) &brr, 8);

	printf("0x%lX\t0x%lX\n", (size_t) arr, (size_t) brr);
	fflush(0);

	drr = (float *) malloc(8 * sizeof(float));

	a = *(__m256 *) arr;
	b = *(__m256 *) brr;

	__m256 c = _mm256_dp_ps(a, b, 0xF1);

	crr = (float *) &c;
	
	drr[0] = fakeOp(arr + 0, brr + 0);
	drr[4] = fakeOp(arr + 4, brr + 4);

	printf("a\t b\t| \t avx2\t x86\n\n");

	for(int i=0; i<8; i++)
		printf("%+0.0f, \t%+0.0f\t|\t %+0.0f, \t%+0.0f\n", arr[i], brr[i], crr[i], drr[i]);

	return 0;
}