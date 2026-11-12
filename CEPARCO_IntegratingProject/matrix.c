#include "matrix.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int matrix_create(Matrix *matrix, size_t rows, size_t cols)
{
    size_t element_count;
    float *data;

    if (matrix == NULL || rows == 0 || cols == 0) {
        return 0;
    }

    if (rows > SIZE_MAX / cols) {
        return 0;
    }

    element_count = rows * cols;
    if (element_count > SIZE_MAX / sizeof(*data)) {
        return 0;
    }

    data = (float *)malloc(element_count * sizeof(*data));
    if (data == NULL) {
        return 0;
    }

    matrix->rows = rows;
    matrix->cols = cols;
    matrix->data = data;
    return 1;
}

void matrix_destroy(Matrix *matrix)
{
    if (matrix == NULL) {
        return;
    }

    free(matrix->data);
    matrix->rows = 0;
    matrix->cols = 0;
    matrix->data = NULL;
}

void matrix_fill_random(Matrix *matrix)
{
    size_t i;
    size_t element_count;

    if (matrix == NULL || matrix->data == NULL) {
        return;
    }

    element_count = matrix->rows * matrix->cols;
    for (i = 0; i < element_count; ++i) {
        matrix->data[i] = (float)rand() / (float)RAND_MAX;
    }
}

void matrix_print(const char *label, const Matrix *matrix)
{
    size_t row;
    size_t col;

    if (matrix == NULL || matrix->data == NULL) {
        printf("%s: unavailable\n", label != NULL ? label : "Matrix");
        return;
    }

    printf("%s (%zu x %zu):\n", label != NULL ? label : "Matrix",
           matrix->rows, matrix->cols);

    for (row = 0; row < matrix->rows; ++row) {
        printf("[");
        for (col = 0; col < matrix->cols; ++col) {
            printf(" %8.3f", matrix->data[row * matrix->cols + col]);
        }
        printf(" ]\n");
    }
}
