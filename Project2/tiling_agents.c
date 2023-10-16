#include <stdlib.h>
#include <stdio.h>

#include "mat_init.h"
#include "mat_funcs.h"
#include "tiling_agents.h"

#include <pthread.h> 


extern struct mat_info_t mat1info, mat2info, matRinfo;
extern struct exec_mode_t exec_mode;

static TiledMatProduct_ptr *TiledMatProduct = NULL;

static void SelectTiledMatProductImpl(void) {
	// Select the correct function for TiledMatProduct.
	if(exec_mode.RowMajorMat2)
		if(exec_mode.avx)
			if(exec_mode.type == FLOAT)
				TiledMatProduct = TiledMatProduct_ABt_avx2_float;
			else
				TiledMatProduct = TiledMatProduct_ABt_avx2_short;
		else
			if(exec_mode.type == FLOAT)
				TiledMatProduct = TiledMatProduct_ABt_float;
			else
				TiledMatProduct = TiledMatProduct_ABt_short;
	else
		if(exec_mode.avx)
			if(exec_mode.type == FLOAT)
				TiledMatProduct = TiledMatProduct_AB_avx2_float;
			else
				TiledMatProduct = TiledMatProduct_AB_avx2_short;
		else
			if(exec_mode.type == FLOAT)
				TiledMatProduct = TiledMatProduct_AB_float;
			else
				TiledMatProduct = TiledMatProduct_AB_short;

	return;
}

struct tileProcessorParams {
	int tile_vlen;
	int tile_hlen;
	range mat1_row_issue;
};


// This function is drawn from "void TilingAgent_classic(int, int)"
void *ThreadableTilingAgent(void *threadArgs) {
	range mat1rows, mat2cols, dot_prod_len;
	int tile_vlen, tile_hlen;

	struct tileProcessorParams *args = (struct tileProcessorParams *) threadArgs;

	tile_vlen = args->tile_vlen;
	tile_hlen = args->tile_hlen;
	mat1rows.first = args->mat1_row_issue.first;
	mat1rows.final = args->mat1_row_issue.final;

	for(int j = 0; j < mat2info.num_cols; j += tile_vlen) {			// specifies a column range of matrix 2 for submatrix ti;e
		mat2cols.first = j;
		mat2cols.final = min(mat2cols.first + tile_vlen, mat2info.num_cols);

		for(int k=0; k < mat1info.num_cols; k += tile_hlen) {	// specifies the length of mat1_row and mat2_col to for vector dot-product
			dot_prod_len.first = k;
			dot_prod_len.final = min(dot_prod_len.first + tile_hlen, mat1info.num_cols);

			TiledMatProduct(mat1rows, mat2cols, dot_prod_len);
		}
	}

	return NULL;
}

void ThreadScheduler(int num_threads, int tile_vlen, int tile_hlen) {
	pthread_t thread_id[num_threads];
	struct tileProcessorParams threadArgs[num_threads];

	int row_idx = 0;
	int row_idx_incr = (mat1info.num_rows / num_threads);

	SelectTiledMatProductImpl();		// uses the global exec_mode struct for calculations

	// spawn all threads
	for(int i=0; i < num_threads; i++) {
		threadArgs[i].tile_vlen = tile_vlen;
		threadArgs[i].tile_hlen = tile_hlen;
		threadArgs[i].mat1_row_issue.first = row_idx;
		threadArgs[i].mat1_row_issue.final = min(row_idx + row_idx_incr, mat1info.num_rows-1);

		row_idx += row_idx_incr +1;

		pthread_create(&thread_id[i], NULL, ThreadableTilingAgent, (void *) &threadArgs[i]);
	}

	// Join all threads
	for(int i=0; i < num_threads; i++)
		pthread_join(thread_id[i], NULL);

	return;
}

//////////////////////////////////////////////// LEGACY FUNCTIONS ////////////////////////////////////////////////

/*			Tiling Agent - Intent of Design
-- This function should takes <tile_length, tile_width> as argument, and tiles the matrices accordingly from left to right.
1. The function must smartly tile at the ends of rows and columns and check for boundries
2. The function must call TiledMatProduct_AB or TiledMatProduct_ABt.
3. Tiles for both sub-matrices have same size, i.e., (#R1, #C1) = (#C2, #R2). Reduces complexity.
4. The function must smartly choose the right TiledMatProduct on basis of Matrix B (Transposed or not).

-- The function specifies a certain range of rows from mat1, a certain range of columns from mat2, and the length of dot_product
-- to the TiledMatProduct_AB function

Ref usage of TiledMatProduct:
void TiledMatProduct_AB(range mat1rows, range mat2cols, range dot_prod_len)
*/
static void TilingAgent_original(int tile_vlen, int tile_hlen) {
	range mat1rows, mat2cols, dot_prod_len;

	// Chose the function type
	TiledMatProduct_ptr *TiledMatProduct = NULL;

	if(exec_mode.RowMajorMat2)
		if(exec_mode.avx)
			TiledMatProduct = TiledMatProduct_ABt_avx2_short;
		else
			TiledMatProduct = TiledMatProduct_ABt_short;
	else
		if(exec_mode.avx)
			TiledMatProduct = TiledMatProduct_AB_avx2_short;
		else
			TiledMatProduct = TiledMatProduct_AB_short;

	for(int i=0; i < mat1info.num_rows; i += tile_vlen) {				// specifies a row range of matrix 1 for submatrix ti;e
		mat1rows.first = i;
		mat1rows.final = min(mat1rows.first + tile_vlen, mat1info.num_rows);

		for(int j=0; j < mat2info.num_cols; j += tile_vlen) {			// specifies a column range of matrix 2 for submatrix ti;e
			mat2cols.first = j;
			mat2cols.final = min(mat2cols.first + tile_vlen, mat2info.num_cols);

			for(int k=0; k < mat1info.num_cols; k += tile_hlen) {	// specifies the length of mat1_row and mat2_col to for vector dot-product
				dot_prod_len.first = k;
				dot_prod_len.final = min(dot_prod_len.first + tile_hlen, mat1info.num_cols);

				TiledMatProduct(mat1rows, mat2cols, dot_prod_len);
			}
		}
	}

	return;
}

/*			Tiling Agent (Classic) - Intent of Design
-- Tiling Agent (Original) in as it processes the rows of matrix 1.
-- This one specifies all rows of matrix 1 to be sent to TiledMatProduct_AB.
-- Results in one less loop, not much performance gain/loss.

Ref usage of TiledMatProduct:
void TiledMatProduct_AB(range mat1rows, range mat2cols, range dot_prod_len)
*/

static void TilingAgent_classic(int tile_vlen, int tile_hlen) {
	range mat1rows, mat2cols, dot_prod_len;

	// Chose the function type
	TiledMatProduct_ptr *TiledMatProduct = NULL;

	if(exec_mode.RowMajorMat2)
		if(exec_mode.avx)
			TiledMatProduct = TiledMatProduct_ABt_avx2_short;
		else
			TiledMatProduct = TiledMatProduct_ABt_short;
	else
		if(exec_mode.avx)
			TiledMatProduct = TiledMatProduct_AB_avx2_short;
		else
			TiledMatProduct = TiledMatProduct_AB_short;

	mat1rows.first = 0;
	mat1rows.final = mat1info.num_rows;

	for(int j=0; j < mat2info.num_cols; j += tile_vlen) {			// specifies a column range of matrix 2 for submatrix ti;e
		mat2cols.first = j;
		mat2cols.final = min(mat2cols.first + tile_vlen, mat2info.num_cols);

		for(int k=0; k < mat1info.num_cols; k += tile_hlen) {	// specifies the length of mat1_row and mat2_col to for vector dot-product
			dot_prod_len.first = k;
			dot_prod_len.final = min(dot_prod_len.first + tile_hlen, mat1info.num_cols);

			TiledMatProduct(mat1rows, mat2cols, dot_prod_len);
		}
	} 

	return;
}