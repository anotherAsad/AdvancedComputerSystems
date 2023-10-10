#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>

float arr[8];
float brr[8];
float *crr;
float *drr;


void initializeArrays(float *a, float *b) {
	for(int i=0; i<8; i++) {
		a[i] = rand() & 0xF;
		b[i] = rand() & 0xF;
		// randomly add sign
		a[i] *= (rand() & 0x1) ? -1 : +1;
		b[i] *= (rand() & 0x1) ? -1 : +1;
	}

	return;
}

float fakeOp(float *a, float *b) {
	int x;
	float sum = 0.0;

	for(x=0; x<4; x++) {
		sum += a[x] * b[x];
	}

	return sum;
}

int main() {
	int i;

	drr = (float *) calloc(0, 8 * sizeof(float));
	initializeArrays(arr, brr);

	__m256 a = *((__m256 *)arr);
	__m256 b = *((__m256 *)brr);

	__m256 c = _mm256_dp_ps(a, b, 0xF1);

	crr = (float *) &c;
	
	drr[0] = fakeOp(arr + 0, brr + 0);
	drr[4] = fakeOp(arr + 4, brr + 4);

	printf("a\t b\t| \t avx2\t x86\n\n");

	for(int i=0; i<8; i++)
		printf("%+0.0f, \t%+0.0f\t|\t %+0.0f, \t%+0.0f\n", arr[i], brr[i], crr[i], drr[i]);

	return 0;
}