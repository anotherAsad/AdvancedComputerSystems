#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <immintrin.h>

#include "mat_funcs.h"
#include "mat_init.h"

#define PAGE_BASE	(0x00000000)			// The physical address to be mapped by mmap. Is NULL due to ANON.

extern float **fmat1, **fmat2, **fmatR;

extern struct mat_info_t mat1info, mat2info, matRinfo;

void TiledMatProduct_AB_float(range mat1rows, range mat2cols, range dot_prod_len) {
	float temp;

	for(int row_idx = mat1rows.first; row_idx < mat1rows.final; row_idx++) {
		for(int col_idx = mat2cols.first; col_idx < mat2cols.final; col_idx++) {
			temp = 0;

			for(int i = dot_prod_len.first; i < dot_prod_len.final; i++)
				temp += fmat1[row_idx][i] * fmat2[i][col_idx];
		
			// alleviates writes race conditions when with multiple cores. Reduces dereferencing overhead.
			fmatR[row_idx][col_idx] += temp;
		}
	}

	return;
}

void TiledMatProduct_AB_avx2_float(range mat1rows, range mat2cols, range dot_prod_len) {
	__m256 prod_vec, accum_vec;
	__m256 col_vec, row_vec;

	float *a;

	for(int row_idx = mat1rows.first; row_idx < mat1rows.final; row_idx++) {
		for(int col_idx = mat2cols.first; col_idx < mat2cols.final; col_idx++) {
			accum_vec = _mm256_setzero_ps();

			for(int i = dot_prod_len.first; i < dot_prod_len.final; i+=8) {
				// IMPLS: temp += fmat1[row_idx][i] * fmat2[i][col_idx];
				row_vec = *(__m256 *) (&fmat1[row_idx][i]);		// make row vector. Translates to loada.

				// Fetch the non-contiguously distributed column elements of matrix 2
				col_vec = _mm256_setr_ps(
					fmat2[i+0x0][col_idx], fmat2[i+0x1][col_idx], fmat2[i+0x2][col_idx], fmat2[i+0x3][col_idx],
					fmat2[i+0x4][col_idx], fmat2[i+0x5][col_idx], fmat2[i+0x6][col_idx], fmat2[i+0x7][col_idx]
				);

				prod_vec  = _mm256_dp_ps(row_vec, col_vec, 0xF1);
				accum_vec = _mm256_add_ps(accum_vec, prod_vec);
			}
			
			// accum_vec has the result as {0, 0, 0, psum[1], 0, 0, 0, psum[0]}
			a = (float *) (&accum_vec);
			// alleviates writes race conditions when with multiple cores. Reduces dereferencing overhead.
			fmatR[row_idx][col_idx] += a[0] + a[4];
		}
	}

	return;
}


// Expects matrix B as transposed
void TiledMatProduct_ABt_float(range mat1rows, range mat2cols, range dot_prod_len) {
	float temp;

	for(int row_idx = mat1rows.first; row_idx < mat1rows.final; row_idx++) {
		for(int col_idx = mat2cols.first; col_idx < mat2cols.final; col_idx++) {
			temp = 0;

			for(int i = dot_prod_len.first; i < dot_prod_len.final; i++)
				temp += fmat1[row_idx][i] * fmat2[col_idx][i];
			
			// alleviates writes race conditions when with multiple cores.  Reduces dereferencing overhead.
			fmatR[row_idx][col_idx] += temp;
		}
	}
	
	return;
}

void TiledMatProduct_ABt_avx2_float(range mat1rows, range mat2cols, range dot_prod_len) {
	__m256 prod_vec, accum_vec;
	__m256 col_vec, row_vec;

	float *a;

	for(int row_idx = mat1rows.first; row_idx < mat1rows.final; row_idx++) {
		for(int col_idx = mat2cols.first; col_idx < mat2cols.final; col_idx++) {
			accum_vec = _mm256_setzero_ps();

			for(int i = dot_prod_len.first; i < dot_prod_len.final; i+=8) {
				// IMPLS: temp += fmat1[row_idx][i] * fmat2[i][col_idx];
				row_vec = *(__m256 *) (&fmat1[row_idx][i]);		// make row vector. Translates to loada.
				col_vec = *(__m256 *) (&fmat2[col_idx][i]);		// make col vector. Translates to loada.

				prod_vec  = _mm256_dp_ps(row_vec, col_vec, 0xF1);
				accum_vec = _mm256_add_ps(accum_vec, prod_vec);
			}
			
			// accum_vec has the result as {0, 0, 0, psum[1], 0, 0, 0, psum[0]}
			a = (float *) (&accum_vec);
			// alleviates writes race conditions when with multiple cores. Reduces dereferencing overhead.
			fmatR[row_idx][col_idx] += a[0] + a[4];
		}
	}

	return;
}

// turns rows of matrix 2 in columns of matrix 2
void TransposeMatrix2_float(void) {
	float **brr = (float **) malloc(mat2info.num_cols * sizeof(float *));
	brr[0] = (float *) mmap(NULL, mat2info.num_rows * mat2info.num_cols * sizeof(float) + 32, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, PAGE_BASE);


	// initialize the row pointers for transposed matrix.
	for(int i=1; i<mat2info.num_cols; i++) {
		brr[i] = brr[i-1] + mat2info.num_rows;			// fmat1[i] should have the address of i-th row
	}
	
	// Perform transpose operation
	for(int i=0; i<mat2info.num_rows; i++) {
		for(int j=0; j<mat2info.num_cols; j++) {
			brr[j][i] = fmat2[i][j];
		}
	}
	
	// free fmat2 and assign anew
	munmap(fmat2, mat2info.num_rows * mat2info.num_cols * sizeof(float) + 16);
	free(fmat2);
	
	fmat2  = brr;
	*fmat2 = *brr;
	
	// swap rows and columns.
	size_t temp = mat2info.num_rows;
	mat2info.num_rows = mat2info.num_cols;
	mat2info.num_cols = temp;

	return;
}

void ClearResultMatrix_float(void) {
	memset(fmatR[0], 0, matRinfo.num_rows * matRinfo.num_cols * sizeof(float));
	return;
}


//////////////////////////////////////////////// LEGACY FUNCTIONS ////////////////////////////////////////////////
// Arguments passed through mat_info_t structs
static void MatProduct_AB(void) {
	for(int row_idx = 0; row_idx < matRinfo.num_rows; row_idx++) {			// #cols_mat2
		for(int col_idx = 0; col_idx < matRinfo.num_cols; col_idx++) {		// #rows_mat1
			for(int i=0; i<mat1info.num_cols; i++)									// could also be mat2info.num_cols
				fmatR[row_idx][col_idx] += fmat1[row_idx][i] * fmat2[i][col_idx];
		}
	}

	return;
}

// Arguments passed through mat_info_t structs
static void MatProduct_ABt(void) {									// Expects fmat2 to be transposed
	for(int row_idx = 0; row_idx < matRinfo.num_rows; row_idx++) {
		for(int col_idx = 0; col_idx < matRinfo.num_cols; col_idx++) {
			for(int i=0; i<mat1info.num_cols; i++)									// could also be mat2info.num_cols
				fmatR[row_idx][col_idx] += fmat1[row_idx][i] * fmat2[col_idx][i];
		}
	}

	return;
}
