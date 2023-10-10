#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

#define SUCCESS 1
#define FAILURE 0

float *arr;
float *brr;
float *crr;
float *drr;

struct metadata_matrices {
	int mat1_rows, mat1_cols;
	int mat2_rows, mat2_cols;

	int tile1_rows, tile1_cols;
	int tile2_rows, tile2_cols;

	float **fmat1, **fmat2, **fmatR;
	short **imat1, **imat2, **imatR;
};

int WriteMatrixElemsToFile(FILE *fptr, int length) {
	short num;
	char *str = (char *) malloc(6 * 1024 + 8);
	int strlen;

	// check for failed file open operation.
	if(fptr == NULL) {
		printf("fptr is NULL !!!\n");
		return FAILURE;
	}
	
	// Proceed to write to file
	for(int i=0; i<length; i+=1024) {
		strlen = 0;
		
		// write to file in large chunks to avoid SSD write hits. Don't know if really needed.
		for(int j=0; j<1024; j++) {
			num = (((int)rand()) << 1) >> (16 + 4);		// 12 bit signed number
			strlen += sprintf(str + strlen, "%d ", num);
		}

		fprintf(fptr, "%s ", str);
	}

	printf("File written with %d elements\n", length);

	free(str);
	return SUCCESS;
}

int main() {
	FILE *fptr;
	
	fptr = fopen("matfile1.txt", "w");

	// Init first file
	if(! WriteMatrixElemsToFile(fptr, 10000 * 10000))
		return -1;
	else
		fclose(fptr);

	// Init second file
	fptr = fopen("matfile2.txt", "w");
	
	if(! WriteMatrixElemsToFile(fptr, 10000 * 10000))
		return -1;
	else
		fclose(fptr);

	return 0;
}