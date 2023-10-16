#include <stdio.h>
#include <time.h>

#include "mat_init.h"
#include "mat_funcs.h"
#include "tiling_agents.h"

#define PRINT 1

#define ROW_COUNT 768
#define COL_COUNT 768

extern struct mat_info_t mat1info, mat2info, matRinfo;

extern struct exec_mode_t exec_mode;

// Function pointer to tiling agent
extern TilingAgent_ptr *TilingAgent;

// Expects the exec_mode and transpose_info to have been specified in main
void ExecutionRun(int num_cores, int hlen, int vlen) {
	struct timespec begin, end;
	double time_elapsed;

	switch(exec_mode.type) {
		case FLOAT:
			// from mat_info.h
			InitializeMatrices_ptr *InitializeMatrices = InitializeMatrices_float;
			FreeMatrices_ptr *FreeMatrices = FreeMatrices_float;
			PrintMatrix_ptr *PrintMatrix = PrintMatrix_float;
			// From mat_funcs.h
			TransposeMatrix2_ptr *TransposeMatrix2 = TransposeMatrix2_float;
			ClearResultMatrix_ptr *ClearResultMatrix = ClearResultMatrix_float;
			break;

		case SHORT:
			// from mat_info.h
			InitializeMatrices_ptr *InitializeMatrices = InitializeMatrices_short;
			FreeMatrices_ptr *FreeMatrices = FreeMatrices_short;
			PrintMatrix_ptr *PrintMatrix = PrintMatrix_short;
			// From mat_funcs.h
			TransposeMatrix2_ptr *TransposeMatrix2 = TransposeMatrix2_short;
			ClearResultMatrix_ptr *ClearResultMatrix = ClearResultMatrix_short;

	}

	mat1info.num_rows = ROW_COUNT;
	mat1info.num_cols = COL_COUNT;

	// #rows_mat2 == #cols_mat1
	mat2info.num_rows = COL_COUNT;
	mat2info.num_cols = ROW_COUNT;
	exec_mode.mat2transposed = 0;

	InitializeMatrices();

	printf(
		"\nMatrices Initialized.\n"
		"Matrix Data Types: %s\n Threading: %s\n AVX: %s\n"
		""
		"Tiling for Cache Optimization: "
		"\tMatrix Two in Row Major Order: %s"
		"\tMatrx Tiling"
		"Testing now ...\n\n",
	);

	/////////////////// EXECUTION RUN BEGINS ///////////////////
	ClearResultMatrix();

	clock_gettime(CLOCK_MONOTONIC, &begin);

	if(exec_mode.mat2transposed)
		TransposeMatrix2();

	ThreadScheduler(4, hlen, vlen);
	clock_gettime(CLOCK_MONOTONIC, &end);

	time_elapsed  = (end.tv_sec - begin.tv_sec);
	time_elapsed += (end.tv_nsec - begin.tv_nsec)/1000000000.0;
	printf("Time elapsed: %+0.3lf ms\n", time_elapsed * 1000.0);

}

int main() {
	struct timespec begin, end;
	double time_elapsed;

	exec_mode.avx = ENABLED;
	exec_mode.type = FLOAT;

	int vlen = 64, hlen = 64;

	InitializeMatrices();

	res_rows.first = 0;
	res_rows.final = matRinfo.num_rows;
	res_cols.first = 0;
	res_cols.final = matRinfo.num_cols;
	res_sum_elems.first = 0;
	res_sum_elems.final = mat1info.num_cols;

	printf("\n\nMatrices Initialized. Testing now ...\n\n\n");

	/////////////////////////////////// NAIVE ///////////////////////////////////
	ClearResultMatrix();

	clock_gettime(CLOCK_MONOTONIC, &begin);
	ThreadScheduler(4, mat1info.num_rows, mat1info.num_cols);
	clock_gettime(CLOCK_MONOTONIC, &end);

	time_elapsed  = (end.tv_sec - begin.tv_sec);
	time_elapsed += (end.tv_nsec - begin.tv_nsec)/1000000000.0;
	printf("Time elapsed (Naive): %+0.3lf ms\n", time_elapsed * 1000.0);

	if(PRINT)
		PrintMatrix(0b100);
}

// Tests transposed AVX
int floatmain() {
	struct timespec begin, end;
	double time_elapsed;
	range res_rows, res_cols, res_sum_elems;

	exec_mode.avx = ENABLED;
	exec_mode.type = FLOAT;

	InitializeMatrices_ptr *InitializeMatrices = InitializeMatrices_float;
	FreeMatrices_ptr *FreeMatrices = FreeMatrices_float;
	PrintMatrix_ptr *PrintMatrix = PrintMatrix_float;

	int vlen = 64, hlen = 64;

	InitializeMatrices();

	res_rows.first = 0;
	res_rows.final = matRinfo.num_rows;
	res_cols.first = 0;
	res_cols.final = matRinfo.num_cols;
	res_sum_elems.first = 0;
	res_sum_elems.final = mat1info.num_cols;

	printf("\n\nMatrices Initialized. Testing now ...\n\n\n");

	TransposeMatrix2_ptr *TransposeMatrix2 = TransposeMatrix2_float;
	ClearResultMatrix_ptr *ClearResultMatrix = ClearResultMatrix_float;

	/////////////////////////////////// NAIVE ///////////////////////////////////
	ClearResultMatrix();

	clock_gettime(CLOCK_MONOTONIC, &begin);
	ThreadScheduler(4, mat1info.num_rows, mat1info.num_cols);
	clock_gettime(CLOCK_MONOTONIC, &end);

	time_elapsed  = (end.tv_sec - begin.tv_sec);
	time_elapsed += (end.tv_nsec - begin.tv_nsec)/1000000000.0;
	printf("Time elapsed (Naive): %+0.3lf ms\n", time_elapsed * 1000.0);

	if(PRINT)
		PrintMatrix(0b100);
	/////////////////////////////////// TRANSPOSED ///////////////////////////////////
	ClearResultMatrix();

	clock_gettime(CLOCK_MONOTONIC, &begin);
	TransposeMatrix2();
	ThreadScheduler(4, mat1info.num_rows, mat1info.num_cols);
	clock_gettime(CLOCK_MONOTONIC, &end);

	TransposeMatrix2();

	time_elapsed  = (end.tv_sec - begin.tv_sec);
	time_elapsed += (end.tv_nsec - begin.tv_nsec)/1000000000.0;
	printf("Time elapsed (Transposed): %+0.3lf ms\n", time_elapsed * 1000.0);

	if(PRINT)
		PrintMatrix(0b100);

	FreeMatrices();

	return 0;
}


// Tests transposed AVX
int short_main() {
	struct timespec begin, end;
	double time_elapsed;
	range res_rows, res_cols, res_sum_elems;

	exec_mode.avx = DISABLED;
	exec_mode.type = SHORT;

	InitializeMatrices_ptr *InitializeMatrices = InitializeMatrices_short;
	FreeMatrices_ptr *FreeMatrices = FreeMatrices_short;
	PrintMatrix_ptr *PrintMatrix = PrintMatrix_short;

	int vlen = 64, hlen = 64;

	InitializeMatrices();

	res_rows.first = 0;
	res_rows.final = matRinfo.num_rows;
	res_cols.first = 0;
	res_cols.final = matRinfo.num_cols;
	res_sum_elems.first = 0;
	res_sum_elems.final = mat1info.num_cols;

	printf("\n\nMatrices Initialized. Testing now ...\n\n\n");

	TransposeMatrix2_ptr *TransposeMatrix2 = TransposeMatrix2_short;
	ClearResultMatrix_ptr *ClearResultMatrix = ClearResultMatrix_short;
	/////////////////////////////////// NAIVE ///////////////////////////////////
	ClearResultMatrix();

	clock_gettime(CLOCK_MONOTONIC, &begin);
	TransposeMatrix2();
	ThreadScheduler(4, mat1info.num_rows, mat1info.num_cols);
	clock_gettime(CLOCK_MONOTONIC, &end);

	TransposeMatrix2();

	time_elapsed  = (end.tv_sec - begin.tv_sec);
	time_elapsed += (end.tv_nsec - begin.tv_nsec)/1000000000.0;
	printf("Time elapsed (No Optimizations): %+0.3lf ms\n", time_elapsed * 1000.0);

	if(PRINT)
		PrintMatrix(0b100);

	/////////////////////////////////// AVX ///////////////////////////////////
	ClearResultMatrix();

	exec_mode.avx = ENABLED;

	clock_gettime(CLOCK_MONOTONIC, &begin);
	TransposeMatrix2();
	ThreadScheduler(4, mat1info.num_rows, mat1info.num_cols);
	clock_gettime(CLOCK_MONOTONIC, &end);

	TransposeMatrix2();

	time_elapsed  = (end.tv_sec - begin.tv_sec);
	time_elapsed += (end.tv_nsec - begin.tv_nsec)/1000000000.0;
	printf("Time elapsed (No Optimizations): %+0.3lf ms\n", time_elapsed * 1000.0);

	if(PRINT)
		PrintMatrix(0b100);


	FreeMatrices();

	return 0;
}