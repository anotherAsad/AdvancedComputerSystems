typedef struct {int first; int final;} range;

////////////////////////// SHORT FUNCTIONS DECL //////////////////////////

void TiledMatProduct_AB_short(range mat1rows, range mat2cols, range dot_prod_len);
void TiledMatProduct_AB_avx2_short(range mat1rows, range mat2cols, range dot_prod_len);
void TiledMatProduct_ABt_short(range mat1rows, range mat2cols, range dot_prod_len);
void TiledMatProduct_ABt_avx2_short(range mat1rows, range mat2cols, range dot_prod_len);

void TransposeMatrix2_short(void);
void ClearResultMatrix_short(void);

////////////////////////// FLAOT FUNCTIONS DECL //////////////////////////

void TiledMatProduct_AB_float(range mat1rows, range mat2cols, range dot_prod_len);
void TiledMatProduct_AB_avx2_float(range mat1rows, range mat2cols, range dot_prod_len);
void TiledMatProduct_ABt_float(range mat1rows, range mat2cols, range dot_prod_len);
void TiledMatProduct_ABt_avx2_float(range mat1rows, range mat2cols, range dot_prod_len);


void TransposeMatrix2_float(void);
void ClearResultMatrix_float(void);

////////////////////////// FUNC PTR DECL //////////////////////////

typedef void TiledMatProduct_ptr(range mat1rows, range mat2cols, range tile);
typedef void TransposeMatrix2_ptr(void);
typedef void ClearResultMatrix_ptr(void);

