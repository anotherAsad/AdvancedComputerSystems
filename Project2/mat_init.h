#define min(x,y) (((x) < (y)) ? (x) : (y))

// containts metadata for matrices
struct mat_info_t {
	size_t num_rows, num_cols;
};

struct exec_mode_t {
	char type;
	char RowMajorMat2;
	char avx;
	int  num_cores;
	int  tile_hlen, tile_vlen;
};

void InitializeMatrices_short(void);
void InitializeMatrices_float(void);
typedef void InitializeMatrices_ptr(void);

void FreeMatrices_short(void);
void FreeMatrices_float(void);
typedef void FreeMatrices_ptr(void);

void PrintMatrix_short(char);
void PrintMatrix_float(char);
typedef void PrintMatrix_ptr(char);