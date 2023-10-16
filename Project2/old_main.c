#include <stdio.h>
#include <time.h>

#include "mat_init.h"
#include "mat_funcs.h"
#include "tiling_agents.h"

#define PRINT 1

extern struct mat_info_t mat1info, mat2info, matRinfo;

extern struct exec_mode_t exec_mode;

// Function pointer to tiling agent
extern TilingAgent_ptr *TilingAgent;

// Tests untransposed AVX
/*
int main() {
	struct timespec begin, end;
	double time_elapsed;
	range res_rows, res_cols, res_sum_elems;

	initialize_matrices();

	int vlen = 64, hlen = 64;


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
	TiledMatProduct_AB(res_rows, res_cols, res_sum_elems);
	clock_gettime(CLOCK_MONOTONIC, &end);

	time_elapsed  = (end.tv_sec - begin.tv_sec);
	time_elapsed += (end.tv_nsec - begin.tv_nsec)/1000000000.0;
	printf("Time elapsed (No Optimizations): %+0.3lf ms\n", time_elapsed * 1000.0);

	if(PRINT)
		PrintMatrix(0b100);

	/////////////////////////////////// AVX ///////////////////////////////////
	ClearResultMatrix();

	clock_gettime(CLOCK_MONOTONIC, &begin);
	TiledMatProduct_AB_avx2(res_rows, res_cols, res_sum_elems);
	clock_gettime(CLOCK_MONOTONIC, &end);

	time_elapsed  = (end.tv_sec - begin.tv_sec);
	time_elapsed += (end.tv_nsec - begin.tv_nsec)/1000000000.0;
	printf("Time elapsed (No Optimizations): %+0.3lf ms\n", time_elapsed * 1000.0);

	if(PRINT)
		PrintMatrix(0b100);
}
*/

// Tests transposed AVX
int main() {
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

/*
int main() {
	struct timespec begin, end;
	double time_elapsed;
	range res_rows, res_cols, res_sum_elems;

	initialize_matrices();

	int vlen = 100, hlen = 100;


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
	TiledMatProduct_AB(res_rows, res_cols, res_sum_elems);
	clock_gettime(CLOCK_MONOTONIC, &end);

	time_elapsed  = (end.tv_sec - begin.tv_sec);
	time_elapsed += (end.tv_nsec - begin.tv_nsec)/1000000000.0;
	printf("Time elapsed (No Optimizations): %+0.3lf ms\n", time_elapsed * 1000.0);

	if(PRINT)
		PrintMatrix(0b100);

	/////////////////////////////////// TRANSPOSED ///////////////////////////////////
	ClearResultMatrix();

	// Clock the execution
	clock_gettime(CLOCK_MONOTONIC, &begin);
	TransposeMatrix2();
	TiledMatProduct_ABt(res_rows, res_cols, res_sum_elems);
	clock_gettime(CLOCK_MONOTONIC, &end);

	// Tranpose back to original
	TransposeMatrix2();
 
	time_elapsed  = (end.tv_sec - begin.tv_sec);
	time_elapsed += (end.tv_nsec - begin.tv_nsec)/1000000000.0;
	printf("Time elapsed (Mat2 Transposed): %+0.3lf ms\n", time_elapsed * 1000.0);

	if(PRINT)
		PrintMatrix(0b100);

	/////////////////////////////////// TILED ///////////////////////////////////
	ClearResultMatrix();

	clock_gettime(CLOCK_MONOTONIC, &begin);
	TilingAgent(vlen, hlen);			// tile_vlen, tile_hlen
	clock_gettime(CLOCK_MONOTONIC, &end);

	time_elapsed  = (end.tv_sec - begin.tv_sec);
	time_elapsed += (end.tv_nsec - begin.tv_nsec)/1000000000.0;
	printf("Time elapsed (Tiled %dx%d, Trnspsd: %d): %+0.3lf ms\n", vlen, hlen, mat2info.transposed, time_elapsed * 1000.0);

	if(PRINT)
		PrintMatrix(0b100);

	/////////////////////////////////// TRANSPOSED TILED ///////////////////////////////////
	ClearResultMatrix();

	clock_gettime(CLOCK_MONOTONIC, &begin);
	TransposeMatrix2();
	TilingAgent(vlen, hlen);			// tile_vlen, tile_hlen
	clock_gettime(CLOCK_MONOTONIC, &end);

	time_elapsed  = (end.tv_sec - begin.tv_sec);
	time_elapsed += (end.tv_nsec - begin.tv_nsec)/1000000000.0;
	printf("Time elapsed (Tiled %dx%d, Trnspsd: %d): %+0.3lf ms\n", vlen, hlen, mat2info.transposed, time_elapsed * 1000.0);

	TransposeMatrix2();

	if(PRINT)
		PrintMatrix(0b100);

	/////////////////////////////////// THREADED TILED ///////////////////////////////////
	ClearResultMatrix();

	clock_gettime(CLOCK_MONOTONIC, &begin);
	ThreadScheduler(4, vlen, hlen);		// int num_threads, int tile_vlen, int tile_hlen
	clock_gettime(CLOCK_MONOTONIC, &end);

	time_elapsed  = (end.tv_sec - begin.tv_sec);
	time_elapsed += (end.tv_nsec - begin.tv_nsec)/1000000000.0;
	printf("Time elapsed (Threaded Tiled %dx%d, Trnspsd: %d): %+0.3lf ms\n", vlen, hlen, mat2info.transposed, time_elapsed * 1000.0);

	if(PRINT)
		PrintMatrix(0b100);

	/////////////////////////////////// THREADED TRANSPOSED TILED ///////////////////////////////////
	ClearResultMatrix();

	clock_gettime(CLOCK_MONOTONIC, &begin);
	TransposeMatrix2();
	ThreadScheduler(4, vlen, hlen);		// int num_threads, int tile_vlen, int tile_hlen
	clock_gettime(CLOCK_MONOTONIC, &end);

	time_elapsed  = (end.tv_sec - begin.tv_sec);
	time_elapsed += (end.tv_nsec - begin.tv_nsec)/1000000000.0;
	printf("Time elapsed (Threaded Tiled %dx%d, Trnspsd: %d): %+0.3lf ms\n", vlen, hlen, mat2info.transposed, time_elapsed * 1000.0);

	TransposeMatrix2();

	if(PRINT)
		PrintMatrix(0b100);

	return 0;
}
*/