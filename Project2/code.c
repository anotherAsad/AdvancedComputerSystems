#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "initialize_short_matrices.h"

#define PRINT 0

extern short **imat1, **imat2;
extern int   **imatR;
extern struct mat_info_t mat1info, mat2info, matRinfo;

// Arguments passed through mat_info_t structs
void MatrixMultiply_NoOptimization(void) {
	for(int res_v = 0; res_v < mat1info.rows; res_v++) {
		for(int res_h = 0; res_h < mat2info.cols; res_h++) {
			for(int i=0; i<mat1info.rows; i++)									// could also be mat2info.cols
				imatR[res_v][res_h] += imat1[res_v][i] * imat2[i][res_h];
		}
	}

	return;
}

// Arguments passed through mat_info_t structs
void MatrixMultiply_PreTransposed(void) {									// Expects imat2 to be transposed
	for(int res_v = 0; res_v < mat1info.rows; res_v++) {
		for(int res_h = 0; res_h < mat2info.cols; res_h++) {
			for(int i=0; i<mat1info.rows; i++)									// could also be mat2info.cols
				imatR[res_v][res_h] += imat1[res_v][i] * imat2[res_h][i];
		}
	}

	return;
}

// turns rows of matrix 2 in columns of matrix 2
void TransposeMatrix2(void) {
	// 1. Make a new matrix in memory
	short **brr = (short **) malloc(mat2info.cols * sizeof(short *));
	*brr = (short  *) malloc(mat2info.cols * mat2info.rows * sizeof(short));

	// initialize the row pointers for transposed matrix.
	for(int i=1; i<mat2info.cols; i++)
		brr[i] = brr[i-1] + mat2info.rows;			// imat1[i] should have the address of i-th row

	// Perform transpose operation
	for(int i=0; i<mat2info.rows; i++) {
		for(int j=0; j<mat2info.cols; j++) {
			brr[j][i] = imat2[i][j];
		}
	}

	// free imat2
	free(imat2[0]);
	free(imat2);

	// assign imat2 to brr
	*imat2 = *brr;
	imat2  = brr;

	// swap rows and columns. Flip transpose flag.
	size_t temp = mat2info.rows;
	mat2info.rows = mat2info.cols;
	mat2info.cols = temp;
	mat2info.transposed = !mat2info.transposed;

	return;
}

void CleanResultMatrix(void) {
	memset(imatR[0], 0, matRinfo.rows * matRinfo.cols * sizeof(int));
	return;
}

int main() {
	clock_t begin, end;
	double time_elapsed;

	initialize_matrices();

	begin = clock();
	MatrixMultiply_NoOptimization();
	end = clock();

	time_elapsed = (double) (end - begin)/CLOCKS_PER_SEC;
	printf("Time elapsed (No Optimizations): %+0.3lf ms\n\n", time_elapsed*1000.0);

	if(PRINT)
		PrintMatrix(0b100);
	
	// Test: Transpose and multiply
	CleanResultMatrix();
	begin = clock();
	TransposeMatrix2();
	MatrixMultiply_PreTransposed();
	end = clock();

	time_elapsed = (double) (end - begin)/CLOCKS_PER_SEC;
	printf("Time elapsed (Mat2 Transposed): %+0.3lf ms\n\n", time_elapsed*1000.0);

	if(PRINT)
		PrintMatrix(0b100);

	return 0;
}