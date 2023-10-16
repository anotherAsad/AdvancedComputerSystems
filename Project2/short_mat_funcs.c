#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <immintrin.h>

#include "mat_funcs.h"
#include "mat_init.h"

#define PAGE_BASE	(0x00000000)			// The physical address to be mapped by mmap. Is NULL due to ANON.

extern short **imat1, **imat2;
extern int   **imatR;

extern struct mat_info_t mat1info, mat2info, matRinfo;

void TiledMatProduct_AB_short(range mat1rows, range mat2cols, range dot_prod_len) {
	int temp;

	for(int row_idx = mat1rows.first; row_idx < mat1rows.final; row_idx++) {
		for(int col_idx = mat2cols.first; col_idx < mat2cols.final; col_idx++) {
			temp = 0;

			for(int i = dot_prod_len.first; i < dot_prod_len.final; i++)
				temp += imat1[row_idx][i] * imat2[i][col_idx];
		
			// alleviates writes race conditions when with multiple cores. Reduces dereferencing overhead.
			imatR[row_idx][col_idx] += temp;
		}
	}

	return;
}

void TiledMatProduct_AB_avx2_short(range mat1rows, range mat2cols, range dot_prod_len) {
	__m256i prod_vec, accum_vec;	// these vecs have 8 ints that needed to be horizontally added at the end
	__m256i col_vec, row_vec;
	__m256i hsum_accum;
	__m256i zero_vec = _mm256_setzero_si256();

	int a, b;

	for(int row_idx = mat1rows.first; row_idx < mat1rows.final; row_idx++) {
		for(int col_idx = mat2cols.first; col_idx < mat2cols.final; col_idx++) {
			accum_vec = _mm256_setzero_si256();

			for(int i = dot_prod_len.first; i < dot_prod_len.final; i += 16) {
				// PERFORMS: temp += imat1[row_idx][i] * imat2[i][col_idx];
				row_vec = *(__m256i *) (&imat1[row_idx][i]);		// make row vector. Translates to loada

				// Fetch the non-contiguously distributed column elements of matrix 2
				col_vec = _mm256_setr_epi16(
					imat2[i+0x0][col_idx], imat2[i+0x1][col_idx], imat2[i+0x2][col_idx], imat2[i+0x3][col_idx],
					imat2[i+0x4][col_idx], imat2[i+0x5][col_idx], imat2[i+0x6][col_idx], imat2[i+0x7][col_idx],
					imat2[i+0x8][col_idx], imat2[i+0x9][col_idx], imat2[i+0xA][col_idx], imat2[i+0xB][col_idx],
					imat2[i+0xC][col_idx], imat2[i+0xD][col_idx], imat2[i+0xE][col_idx], imat2[i+0xF][col_idx]
				);

				// prod_vec[7:0] = hsum(row_vec[7:0][1:0] * col_vec[7:0][1:0], sum_dimm[2])
				prod_vec = _mm256_madd_epi16(row_vec, col_vec);
				accum_vec = _mm256_add_epi32(accum_vec, prod_vec);
			}
			
			// PERFOMS:	imatR[row_idx][col_idx] += temp;

			// accum_vec = {psum[7], psum[6], psum[5], psum[4], psum[3], psum[2], psum[1], psum[0]}
			hsum_accum = _mm256_hadd_epi32(accum_vec, zero_vec);
			// hsum_accum = {0, 0, psum[7, 6], psum[5, 4], 0, 0, psum[3, 2], psum[1, 0]}
			hsum_accum = _mm256_hadd_epi32(hsum_accum, zero_vec);
			// hsum_accum = {0, 0, 0, psum[7, 6, 5, 4], 0, 0, 0, psum[3, 2, 1, 0]}

			// Extract 0-th and 4-th int from hsum_accum. Add them and put them into imatR
			a = _mm256_extract_epi32(hsum_accum, 0);
			b = _mm256_extract_epi32(hsum_accum, 4);

			imatR[row_idx][col_idx] += (a + b);
		}
	}
	
	return;
}


// Expects matrix B as transposed
void TiledMatProduct_ABt_short(range mat1rows, range mat2cols, range dot_prod_len) {
	int temp;

	for(int row_idx = mat1rows.first; row_idx < mat1rows.final; row_idx++) {
		for(int col_idx = mat2cols.first; col_idx < mat2cols.final; col_idx++) {
			temp = 0;

			for(int i = dot_prod_len.first; i < dot_prod_len.final; i++)
				temp += imat1[row_idx][i] * imat2[col_idx][i];
			
			// alleviates writes race conditions when with multiple cores.  Reduces dereferencing overhead.
			imatR[row_idx][col_idx] += temp;
		}
	}
	
	return;
}

void TiledMatProduct_ABt_avx2_short(range mat1rows, range mat2cols, range dot_prod_len) {
	__m256i prod_vec, accum_vec;	// these vecs have 8 ints that needed to be horizontally added at the end
	__m256i col_vec, row_vec;
	__m256i hsum_accum;
	__m256i zero_vec = _mm256_setzero_si256();

	int a, b;

	for(int row_idx = mat1rows.first; row_idx < mat1rows.final; row_idx++) {
		for(int col_idx = mat2cols.first; col_idx < mat2cols.final; col_idx++) {
			accum_vec = _mm256_setzero_si256();

			for(int i = dot_prod_len.first; i < dot_prod_len.final; i += 16) {
				// PERFORMS: temp += imat1[row_idx][i] * imat2[col_idx][i];
				row_vec = *(__m256i *) (&imat1[row_idx][i]);		// make row vector. Translates to loada.
				col_vec = *(__m256i *) (&imat2[col_idx][i]);		// make col vector. Translates to loada.

				// prod_vec[7:0] = hsum(row_vec[7:0][1:0] * col_vec[7:0][1:0], sum_dimm[2])
				prod_vec = _mm256_madd_epi16(row_vec, col_vec);
				accum_vec = _mm256_add_epi32(accum_vec, prod_vec);
			}
			
			// PERFOMS:	imatR[row_idx][col_idx] += temp;

			// accum_vec = {psum[7], psum[6], psum[5], psum[4], psum[3], psum[2], psum[1], psum[0]}
			hsum_accum = _mm256_hadd_epi32(accum_vec, zero_vec);
			// hsum_accum = {0, 0, psum[7, 6], psum[5, 4], 0, 0, psum[3, 2], psum[1, 0]}
			hsum_accum = _mm256_hadd_epi32(hsum_accum, zero_vec);
			// hsum_accum = {0, 0, 0, psum[7, 6, 5, 4], 0, 0, 0, psum[3, 2, 1, 0]}

			// Extract 0-th and 4-th int from hsum_accum. Add them and put them into imatR
			a = _mm256_extract_epi32(hsum_accum, 0);
			b = _mm256_extract_epi32(hsum_accum, 4);

			imatR[row_idx][col_idx] += (a + b);
		}
	}
	
	return;
}

// turns rows of matrix 2 in columns of matrix 2
void TransposeMatrix2_short(void) {
	short **brr = (short **) malloc(mat2info.num_cols * sizeof(short *));
	brr[0] = (short *) mmap(NULL, mat2info.num_rows * mat2info.num_cols * sizeof(short) + 32, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, PAGE_BASE);


	// initialize the row pointers for transposed matrix.
	for(int i=1; i<mat2info.num_cols; i++) {
		brr[i] = brr[i-1] + mat2info.num_rows;			// imat1[i] should have the address of i-th row
	}
	
	// Perform transpose operation
	for(int i=0; i<mat2info.num_rows; i++) {
		for(int j=0; j<mat2info.num_cols; j++) {
			brr[j][i] = imat2[i][j];
		}
	}
	
	// free imat2 and assign anew
	munmap(imat2, mat2info.num_rows * mat2info.num_cols * sizeof(short) + 16);
	free(imat2);
	
	imat2  = brr;
	*imat2 = *brr;
	
	// swap rows and columns.
	size_t temp = mat2info.num_rows;
	mat2info.num_rows = mat2info.num_cols;
	mat2info.num_cols = temp;

	return;
}

void ClearResultMatrix_short(void) {
	memset(imatR[0], 0, matRinfo.num_rows * matRinfo.num_cols * sizeof(int));
	return;
}


//////////////////////////////////////////////// LEGACY FUNCTIONS ////////////////////////////////////////////////
// Arguments passed through mat_info_t structs
static void MatProduct_AB(void) {
	for(int row_idx = 0; row_idx < matRinfo.num_rows; row_idx++) {			// #cols_mat2
		for(int col_idx = 0; col_idx < matRinfo.num_cols; col_idx++) {		// #rows_mat1
			for(int i=0; i<mat1info.num_cols; i++)									// could also be mat2info.num_cols
				imatR[row_idx][col_idx] += imat1[row_idx][i] * imat2[i][col_idx];
		}
	}

	return;
}

// Arguments passed through mat_info_t structs
static void MatProduct_ABt(void) {									// Expects imat2 to be transposed
	for(int row_idx = 0; row_idx < matRinfo.num_rows; row_idx++) {
		for(int col_idx = 0; col_idx < matRinfo.num_cols; col_idx++) {
			for(int i=0; i<mat1info.num_cols; i++)									// could also be mat2info.num_cols
				imatR[row_idx][col_idx] += imat1[row_idx][i] * imat2[col_idx][i];
		}
	}

	return;
}

