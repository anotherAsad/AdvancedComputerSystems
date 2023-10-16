#include <stdio.h>
#include <time.h>

#include "mat_init.h"
#include "mat_funcs.h"
#include "tiling_agents.h"

#define PRINT 0

extern struct mat_info_t mat1info, mat2info, matRinfo;
extern struct exec_mode_t exec_mode;

static FILE *fptr = NULL;


// Expects the exec_mode and transpose_info to have been specified in main
double TimedExecution() {
	struct timespec begin, end;
	double time_elapsed;

	printf("******************************************************\n");

	InitializeMatrices_ptr *InitializeMatrices;
	FreeMatrices_ptr *FreeMatrices;
	PrintMatrix_ptr *PrintMatrix;
	TransposeMatrix2_ptr *TransposeMatrix2;
	ClearResultMatrix_ptr *ClearResultMatrix;

	switch(exec_mode.type) {
		case FLOAT:
			// from mat_info.h
			InitializeMatrices = InitializeMatrices_float;
			FreeMatrices = FreeMatrices_float;
			PrintMatrix = PrintMatrix_float;
			// From mat_funcs.h
			TransposeMatrix2 = TransposeMatrix2_float;
			ClearResultMatrix = ClearResultMatrix_float;
			break;

		case SHORT:
			// from mat_info.h
			InitializeMatrices = InitializeMatrices_short;
			FreeMatrices = FreeMatrices_short;
			PrintMatrix = PrintMatrix_short;
			// From mat_funcs.h
			TransposeMatrix2 = TransposeMatrix2_short;
			ClearResultMatrix = ClearResultMatrix_short;
			break;
		
		default:
			printf("!!! Unknown Type. Exiting.\n");
			return 0.0;
			break;
	}

	InitializeMatrices();

	/////////////////////// INFO PRINTING SECTION ///////////////////////

	printf("\nMatrices Initialized. Execution Run Info:\n\n");

	printf(
		"Matrix Data Types: %s\n"
		"AVX: %s\n"
		"Threading: %s (Thread Count = %d)\n"
		"Cache Optimizations\n"
		"\tMatrix 2 in Row Major Order: %s\n",
		(exec_mode.type == FLOAT)  ? "FLOAT" : "SHORT",
		(exec_mode.avx == ENABLED) ? "ENABLED" : "DISABLED",
		(exec_mode.num_cores == 1) ? "No" : "Yes", (exec_mode.num_cores),
		(exec_mode.RowMajorMat2)   ? "YES" : "NO"
	);

	if(exec_mode.tile_hlen == mat2info.num_cols && exec_mode.tile_vlen == mat1info.num_rows)
		printf("\tMatrix Tiling: NO\n");
	else
		printf("\tMatrix Tiling: YES\t(%dx%d)\n", exec_mode.tile_hlen, exec_mode.tile_vlen);

	printf("\nTesting now ...\n\n");

	/////////////////// EXECUTION RUN BEGINS ///////////////////
	ClearResultMatrix();
	
	// Time the execution: Get Starting Timestamp
	clock_gettime(CLOCK_MONOTONIC, &begin);
	// Transpose Matrix 2 if Row Major Form is to be used.
	if(exec_mode.RowMajorMat2)
		TransposeMatrix2();
	// Initialize the Execution
	ThreadScheduler(exec_mode.num_cores, exec_mode.tile_hlen, exec_mode.tile_vlen);
	// Time the execution: Get Ending Timestamp
	clock_gettime(CLOCK_MONOTONIC, &end);

	time_elapsed  = (end.tv_sec - begin.tv_sec);
	time_elapsed += (end.tv_nsec - begin.tv_nsec)/1000000000.0;

	// PrintMatrix(0b100);
	printf("Time taken: %+0.3lf ms\n", time_elapsed * 1000.0);

	FreeMatrices();

	printf("******************************************************\n");

	return time_elapsed*1000.0;
}

int ExecutionIterator(int ROW_COUNT, int COL_COUNT) {
	int runCount = 0;
	double time_elapsed = 0.0;

	mat1info.num_rows = 16*(ROW_COUNT/16);
	mat1info.num_cols = 16*(COL_COUNT/16);
	// #rows_mat2 == #cols_mat1
	mat2info.num_rows = 16*(COL_COUNT/16);
	mat2info.num_cols = 16*(ROW_COUNT/16);

	int tileLens[] = {64, mat1info.num_rows};

	// Open a file for recording outputs
	fptr = fopen("output.csv", "a");

	if(fptr == NULL) {
		printf("Failed to open file for output. Exiting...\n");
		return -1;
	}
	
	//fprintf(fptr, "Run_Count, MatDim_X, MatDim_Y, Data_Type, Mat_2_Row_Major, AVX, Thread_Count, Tile_Length, Execution_Time_ms\n");

	for(char type = 0; type < 1; type++){
		for(char trnsps = 1; trnsps < 2; trnsps++){
			for(char avx=0; avx<2; avx++){
				for(int num_cores=1; num_cores <= 8; num_cores *= 4){
					for(int tileLenIdx = 0; tileLenIdx < 2;  tileLenIdx++){
						// Guard for tileLen overflow
						if(tileLens[tileLenIdx] > mat1info.num_rows)
							continue;
						else
							printf("\n\nMat Dim %dx%d\n", ROW_COUNT, ROW_COUNT);

						exec_mode.type = type;
						exec_mode.RowMajorMat2 = trnsps;
						exec_mode.avx = avx;
						exec_mode.num_cores = num_cores;
						exec_mode.tile_hlen = tileLens[tileLenIdx];
						exec_mode.tile_vlen = tileLens[tileLenIdx];

						fprintf(fptr, "%d, %d, %d, ", runCount++, ROW_COUNT, COL_COUNT);
						fprintf(fptr, "%d, %d, %d, ", exec_mode.type, exec_mode.RowMajorMat2, exec_mode.avx);
						fprintf(
							fptr, "%d, %d, ",
							exec_mode.num_cores, exec_mode.tile_hlen
						);

						time_elapsed = TimedExecution();

						fprintf(fptr, "%+0.3lf\n", time_elapsed);
					}
				}
			}
		}
	}

	fclose(fptr);
	return 0;
}

int main() {
	ExecutionIterator(1000, 1000);
}

//////////////////////////////////////////////// LEGACY FUNCTIONS ////////////////////////////////////////////////
// Tests transposed AVX
int float_main() {
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