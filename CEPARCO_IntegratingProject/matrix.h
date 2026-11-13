#ifndef MATRIXH
#define MATRIXH

#include <stddef.h>

typedef struct Matrix {
    size_t rows;
    size_t cols;
    float *data;
} Matrix;

int matrixCreate(Matrix *matrix, size_t rows, size_t cols);
void matrixDestroy(Matrix *matrix);
void matrixFillRandom(Matrix *matrix);
void matrixPrint(const char *label, const Matrix *matrix);

void matrixMultiplyIjk(const Matrix *a, const Matrix *b, Matrix *c);
void matrixMultiplyIkj(const Matrix *a, const Matrix *b, Matrix *c);
void matrixMultiplyAvx2(const Matrix *a, const Matrix *b, Matrix *c);

int matrixEqualTolerance(
    const Matrix *left,
    const Matrix *right,
    float tolerance
);

#endif
