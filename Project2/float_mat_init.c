#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "mat_init.h"

#define SUCCESS 1
#define FAILURE 0

#define PAGE_BASE	(0x00000000)			// The physical address to be mapped by mmap. Is NULL due to ANON.

/////////////////////////////////////// EXPOSABLES ///////////////////////////////////////
float **fmat1, **fmat2, **fmatR;		// R for results
/////////////////////////////////////////////////////////////////////////////////////////

extern struct mat_info_t mat1info, mat2info, matRinfo;
extern struct exec_mode_t exec_mode;

void InitializeMatrices_float(void) {
	auto void PopulateMatrixFromFile(float *arr, int size);		// subroutine to populate matrices
	auto void PopulateMatrixAtRandom(float *arr, int size);

	// Results has dimensions (#rows, #cols) = (#rows_mat1, #cols_mat2)
	matRinfo.num_rows = mat1info.num_rows;
	matRinfo.num_cols = mat2info.num_cols;

	fmat1  = (float **) malloc(mat1info.num_rows * sizeof(float *));					// an array with the address of rows
	fmat2  = (float **) malloc(mat2info.num_rows * sizeof(float *));					// an array with the address of rows
	fmatR  = (float **) malloc(matRinfo.num_rows * sizeof(float *));					// an array with the address of rows
	// fmat[0] == *fmat points to first float in the contiguous array
	fmat1[0] = (float *) mmap(NULL, mat1info.num_rows * mat1info.num_cols * sizeof(float) + 32, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, PAGE_BASE);
	fmat2[0] = (float *) mmap(NULL, mat2info.num_rows * mat2info.num_cols * sizeof(float) + 32, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, PAGE_BASE);
	fmatR[0] = (float *) mmap(NULL, matRinfo.num_rows * matRinfo.num_cols * sizeof(float) + 32, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, PAGE_BASE);

	// initialize the row pointers for mat1.
	for(int row_idx=1; row_idx<mat1info.num_rows; row_idx++)
		fmat1[row_idx] = fmat1[row_idx-1] + mat1info.num_cols;			// fmat1[i] should have the address of i-th row

	// initialize the row pointers for mat2.
	for(int row_idx=1; row_idx<mat2info.num_rows; row_idx++)
		fmat2[row_idx] = fmat2[row_idx-1] + mat2info.num_cols;			// fmat2[i] should have the address of i-th row

	// initialize the row pointers for matR.
	for(int row_idx=1; row_idx<matRinfo.num_rows; row_idx++)
		fmatR[row_idx] = fmatR[row_idx-1] + matRinfo.num_cols;			// fmatR[i] should have the address of i-th row

	printf("0x%lX 0x%lX\n", (size_t) fmatR[0], (size_t) fmatR[1]);

	// read data from files
	FILE *fptr;

	fptr = fopen("matfile1.txt", "r");
	PopulateMatrixFromFile(fmat1[0], mat1info.num_rows * mat1info.num_cols);

	fptr = fopen("matfile2.txt", "r");
	PopulateMatrixFromFile(fmat2[0], mat2info.num_rows * mat2info.num_cols);

	return;

	//////////////////////////////// SUBROUTINES //////////////////////////////
	void PopulateMatrixFromFile(float *arr, int size) {
		static int call_count = 1;
		int ret_val = 0;

		if(fptr == NULL){
			printf("\n!!! NULL pointer passed as fptr !!!\n");
			return;
		}

		// perform linear reads
		for(int i=0; i < size; i++)
			ret_val = fscanf(fptr, "%f ", &arr[i]);

		printf("File read complete. Matrix %d populated\n", call_count++);

		fclose(fptr);
		return;
	}

	void PopulateMatrixAtRandom(float *arr, int size) {
		static int call_count = 1;
		// perform linear reads
		for(int i=0; i < size; i++)
			arr[i] = (float)rand() / 16.0;			// rand is cast first, then shifted by 4

		printf("Matrix %d populated by abs(randoms) < 4096\n", call_count++);

		return;
	}
}

void PrintMatrix_float(char flag) {
	static const int PRINT_LIM = 8;
	// Print 1st matrix
	if(flag & 0x1) {
		printf("\nMatrix 1 (Only %dx%d top-left tile):\n\n", PRINT_LIM, PRINT_LIM);
		
		for(int row_idx=0; row_idx<min(mat1info.num_rows, PRINT_LIM); row_idx++) {
			for(int col_idx=0; col_idx<min(mat1info.num_cols, PRINT_LIM); col_idx++)
				printf("%+0.1f\t", fmat1[row_idx][col_idx]);

			printf("\n\n");
		}

		printf("\nFirst Row:\n");
		// First Row:
		for(int col_idx=0; col_idx<mat1info.num_cols; col_idx++)
			printf("%+0.1f, ", fmat1[0][col_idx]);

		printf("\n");
	}

	// Print 2nd matrix
	if(flag & 0x2) {
		printf("\nMatrix 2 [Trnsps: %d] (Only %dx%d top-left tile):\n\n", exec_mode.RowMajorMat2, PRINT_LIM, PRINT_LIM);
		
		for(int row_idx=0; row_idx<min(mat2info.num_rows, PRINT_LIM); row_idx++) {
			for(int col_idx=0; col_idx<min(mat2info.num_cols, PRINT_LIM); col_idx++)
				printf("%+0.1f\t", fmat2[row_idx][col_idx]);

			printf("\n");
		}

		printf("\nFirst Col:\n");
		// First Col:
		for(int row_idx=0; row_idx<mat2info.num_rows; row_idx++)
			printf("%+0.1f, ", fmat2[row_idx][0]);

		printf("\n");
	}

	// Print 3rd matrix
	if(flag & 0x4) {
		printf("\nMatrix R (Elements divided by 65536 | only %dx%d top-left tile):\n\n", PRINT_LIM, PRINT_LIM);
		
		for(int row_idx=0; row_idx<min(matRinfo.num_rows, PRINT_LIM); row_idx++) {
			for(int col_idx=0; col_idx<min(matRinfo.num_cols, PRINT_LIM); col_idx++)
				printf("%+0.1f\t", fmatR[row_idx][col_idx] / 65536.0);

			printf("\n");
		}
	}

	printf("\n\n");
	return;
}

void FreeMatrices_float(void) {
	munmap(fmat1, mat1info.num_rows * mat1info.num_cols * sizeof(float) + 16);
	free(fmat1);

	munmap(fmat2, mat2info.num_rows * mat2info.num_cols * sizeof(float) + 16);
	free(fmat2);

	munmap(fmatR, matRinfo.num_rows * matRinfo.num_cols * sizeof(float) + 8);
	free(fmatR);

	printf("\nAll Matrices freed\n");

	return;
}