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
short **imat1, **imat2;
int   **imatR;		// R for results

struct mat_info_t mat1info, mat2info, matRinfo;
struct exec_mode_t exec_mode;
/////////////////////////////////////////////////////////////////////////////////////////

void InitializeMatrices_short(void) {
	auto void PopulateMatrixFromFile(short *arr, int size);		// subroutine to populate matrices
	auto void PopulateMatrixAtRandom(short *arr, int size);

	// Results has dimensions (#rows, #cols) = (#rows_mat1, #cols_mat2)
	matRinfo.num_rows = mat1info.num_rows;
	matRinfo.num_cols = mat2info.num_cols;

	imat1  = (short **) malloc(mat1info.num_rows * sizeof(short *));					// an array with the address of rows
	imat2  = (short **) malloc(mat2info.num_rows * sizeof(short *));					// an array with the address of rows
	imatR  = (int   **) malloc(matRinfo.num_rows * sizeof(int   *));					// an array with the address of rows
	// imat[0] == *imat points to first short in the contiguous array
	imat1[0] = (short *) mmap(NULL, mat1info.num_rows * mat1info.num_cols * sizeof(short) + 32, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, PAGE_BASE);
	imat2[0] = (short *) mmap(NULL, mat2info.num_rows * mat2info.num_cols * sizeof(short) + 32, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, PAGE_BASE);
	imatR[0] = (int   *) mmap(NULL, matRinfo.num_rows * matRinfo.num_cols * sizeof(int) + 32, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, PAGE_BASE);

	// initialize the row pointers for mat1.
	for(int row_idx=1; row_idx<mat1info.num_rows; row_idx++)
		imat1[row_idx] = imat1[row_idx-1] + mat1info.num_cols;			// imat1[i] should have the address of i-th row

	// initialize the row pointers for mat2.
	for(int row_idx=1; row_idx<mat2info.num_rows; row_idx++)
		imat2[row_idx] = imat2[row_idx-1] + mat2info.num_cols;			// imat2[i] should have the address of i-th row

	// initialize the row pointers for matR.
	for(int row_idx=1; row_idx<matRinfo.num_rows; row_idx++)
		imatR[row_idx] = imatR[row_idx-1] + matRinfo.num_cols;			// imatR[i] should have the address of i-th row

	printf("0x%lX 0x%lX\n", (size_t) imatR[0], (size_t) imatR[1]);

	// read data from files
	FILE *fptr;

	fptr = fopen("matfile1.txt", "r");
	PopulateMatrixFromFile(imat1[0], mat1info.num_rows * mat1info.num_cols);

	fptr = fopen("matfile2.txt", "r");
	PopulateMatrixFromFile(imat2[0], mat2info.num_rows * mat2info.num_cols);

	return;

	//////////////////////////////// SUBROUTINES //////////////////////////////
	void PopulateMatrixFromFile(short *arr, int size) {
		static int call_count = 1;
		int ret_val = 0;

		if(fptr == NULL){
			printf("\n!!! NULL pointer passed as fptr !!!\n");
			return;
		}

		// perform linear reads
		for(int i=0; i < size; i++)
			ret_val = fscanf(fptr, "%hd ", &arr[i]);

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

void PrintMatrix_short(char flag) {
	static const int PRINT_LIM = 8;
	// Print 1st matrix
	if(flag & 0x1) {
		printf("\nMatrix 1 (Only %dx%d top-left tile):\n\n", PRINT_LIM, PRINT_LIM);
		
		for(int row_idx=0; row_idx<min(mat1info.num_rows, PRINT_LIM); row_idx++) {
			for(int col_idx=0; col_idx<min(mat1info.num_cols, PRINT_LIM); col_idx++)
				printf("%+d\t", imat1[row_idx][col_idx]);

			printf("\n\n");
		}

		printf("\nFirst Row:\n");
		// First Row:
		for(int col_idx=0; col_idx<mat1info.num_cols; col_idx++)
			printf("%+d, ", imat1[0][col_idx]);

		printf("\n");
	}

	// Print 2nd matrix
	if(flag & 0x2) {
		printf("\nMatrix 2 [Trnsps: %d] (Only %dx%d top-left tile):\n\n", exec_mode.mat2transposed, PRINT_LIM, PRINT_LIM);
		
		for(int row_idx=0; row_idx<min(mat2info.num_rows, PRINT_LIM); row_idx++) {
			for(int col_idx=0; col_idx<min(mat2info.num_cols, PRINT_LIM); col_idx++)
				printf("%+d\t", imat2[row_idx][col_idx]);

			printf("\n");
		}

		printf("\nFirst Col:\n");
		// First Col:
		for(int row_idx=0; row_idx<mat2info.num_rows; row_idx++)
			printf("%+d, ", imat2[row_idx][0]);

		printf("\n");
	}

	// Print 3rd matrix
	if(flag & 0x4) {
		printf("\nMatrix R (Elements divided by 65536 | only %dx%d top-left tile):\n\n", PRINT_LIM, PRINT_LIM);
		
		for(int row_idx=0; row_idx<min(matRinfo.num_rows, PRINT_LIM); row_idx++) {
			for(int col_idx=0; col_idx<min(matRinfo.num_cols, PRINT_LIM); col_idx++)
				printf("%+d\t", imatR[row_idx][col_idx] >> 16);

			printf("\n");
		}
	}

	printf("\n\n");
	return;
}

void FreeMatrices_short(void) {
	munmap(imat1, mat1info.num_rows * mat1info.num_cols * sizeof(short) + 16);
	free(imat1);

	munmap(imat2, mat2info.num_rows * mat2info.num_cols * sizeof(short) + 16);
	free(imat2);

	munmap(imatR, matRinfo.num_rows * matRinfo.num_cols * sizeof(int) + 8);
	free(imatR);

	printf("\nAll Matrices freed\n");

	return;
}