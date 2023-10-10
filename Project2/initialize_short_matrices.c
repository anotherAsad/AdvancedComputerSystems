#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

#include "initialize_short_matrices.h"

#define SUCCESS 1
#define FAILURE 0

/////////////////////////////////////// EXPOSABLES ///////////////////////////////////////
short **imat1, **imat2;
int   **imatR;		// R for results
struct mat_info_t mat1info, mat2info, matRinfo;
/////////////////////////////////////////////////////////////////////////////////////////

void initialize_matrices(void) {
	auto void PopulateMatrixFromFile(short *arr, int size);		// subroutine to populate matrices

	mat1info.rows = ROW_COUNT;
	mat1info.cols = COL_COUNT;

	mat2info.rows = ROW_COUNT;
	mat2info.cols = COL_COUNT;
	mat2info.transposed = 0;

	matRinfo.rows = mat2info.cols;
	matRinfo.cols = mat1info.rows;

	imat1  = (short **) malloc(mat1info.rows * sizeof(short *));					// an array with the address of rows
	imat2  = (short **) malloc(mat2info.rows * sizeof(short *));					// an array with the address of rows
	imatR  = (int   **) malloc(matRinfo.rows * sizeof(int   *));					// an array with the address of rows
	// imat[0] == *imat points to first short in the contiguous array
	*imat1 = (short  *) malloc(mat1info.rows * mat1info.cols * sizeof(short));		// an array with dumb shorts.
	*imat2 = (short  *) malloc(mat2info.rows * mat2info.cols * sizeof(short));		// an array with dumb shorts.
	*imatR = (int    *) calloc(matRinfo.rows * matRinfo.cols * sizeof(int), 1);		// an array ints initilized at zero.

	// initialize the row pointers for mat1.
	for(int i=1; i<mat1info.rows; i++)
		imat1[i] = imat1[i-1] + mat1info.cols;			// imat1[i] should have the address of i-th row

	// initialize the row pointers for mat2.
	for(int i=1; i<mat2info.rows; i++)
		imat2[i] = imat2[i-1] + mat2info.cols;			// imat1[i] should have the address of i-th row

	// initialize the row pointers for matR.
	for(int i=1; i<matRinfo.rows; i++)
		imatR[i] = imatR[i-1] + matRinfo.cols;			// imat1[i] should have the address of i-th row

	printf("0x%lX 0x%lX\n", (size_t) imatR[0], (size_t) imatR[1]);

	// read data from files
	FILE *fptr;

	fptr = fopen("matfile1.txt", "r");
	PopulateMatrixFromFile(imat1[0], mat1info.rows * mat1info.cols);

	fptr = fopen("matfile2.txt", "r");
	PopulateMatrixFromFile(imat2[0], mat2info.rows * mat2info.cols);

	return;

	//////////////////////////////// SUBROUTINES //////////////////////////////
	void PopulateMatrixFromFile(short *arr, int size) {
		if(fptr == NULL){
			printf("\n!!! NULL pointer passed as fptr !!!\n");
			return;
		}

		static int call_count = 1;
		// perform linear reads
		for(int i=0; i < size; i++)
			fscanf(fptr, "%hd ", &arr[i]);

		printf("File read complete. Matrix %d populated\n", call_count++);

		fclose(fptr);
		return;
	}

	void PopulateMatrixAtRandom(short *arr, int size) {
		static int call_count = 1;
		// perform linear reads
		for(int i=0; i < size; i++)
			arr[i] = (short)rand() >> 4;			// rand is cast first, then shifted by 4

		printf("Matrix %d populated by abs(randoms) < 4096\n", call_count++);

		return;
	}
}

void PrintMatrix(char flag) {
	auto int min(int, int);
	// Print 1st matrix
	if(flag & 0x1) {
		printf("\nMatrix 1:\n\n");
		
		for(int i=0; i<mat1info.rows; i++) {
			for(int j=0; j<mat1info.cols; j++)
				printf("%+d\t", imat1[i][j]);

			printf("\n");
		}
	}

	// Print 2nd matrix
	if(flag & 0x2) {
		printf("\nMatrix 2:\n\n");
		
		for(int i=0; i<mat2info.rows; i++) {
			for(int j=0; j<mat2info.cols; j++)
				printf("%+d\t", imat2[i][j]);

			printf("\n");
		}
	}

	// Print 3rd matrix
	if(flag & 0x4) {
		printf("\nMatrix R (Elements divided by 65536 | only 16x16 top-left tile):\n\n");
		
		for(int i=0; i<min(matRinfo.rows, 16); i++) {
			for(int j=0; j<min(matRinfo.cols, 16); j++)
				printf("%+d\t", imatR[i][j] >> 16);

			printf("\n");
		}
	}

	printf("\n\n");
	return;

	///////////// SUBROUTINES //////////////
	int min(int a, int b) {
		if(a < b)
			return a;
		else
			return b;
	}
}

void free_matrices(void) {
	free(imat1[0]);
	free(imat1);

	free(imat2[0]);
	free(imat2);

	free(imatR[0]);
	free(imatR);

	printf("\nAll Matrices freed\n");

	return;
}