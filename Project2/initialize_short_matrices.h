#define ROW_COUNT 100
#define COL_COUNT 100

// containts metadata for matrices
struct mat_info_t {
	size_t rows, cols;
	size_t tileLen_x, tileLen_y;
	char transposed;
};

void initialize_matrices(void);
void free_matrices(void);
void PrintMatrix(char);