#ifndef MATRIX_H
#define MATRIX_H

#include <stddef.h>

/* Matrix elements are contiguous in row-major order. */
typedef struct Matrix {
    size_t rows;
    size_t cols;
    float *data;
} Matrix;

/* Initialize to {0}; destroy the matrix before creating it again. */
int matrix_create(Matrix *matrix, size_t rows, size_t cols);
void matrix_destroy(Matrix *matrix);
void matrix_fill_random(Matrix *matrix);
void matrix_print(const char *label, const Matrix *matrix);

void matrix_multiply_ijk(const Matrix *a, const Matrix *b, Matrix *c);
void matrix_multiply_ikj(const Matrix *a, const Matrix *b, Matrix *c);
void matrix_multiply_avx2(const Matrix *a, const Matrix *b, Matrix *c);

int matrix_equal_tolerance(
    const Matrix *left,
    const Matrix *right,
    float tolerance
);

#endif
