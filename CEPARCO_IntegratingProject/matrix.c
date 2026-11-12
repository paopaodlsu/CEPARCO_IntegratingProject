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
    size_t row_slot;
    size_t col_slot;
    size_t row;
    size_t col;
    size_t displayed_rows;
    size_t displayed_cols;

    if (matrix == NULL || matrix->data == NULL) {
        printf("%s: unavailable\n", label != NULL ? label : "Matrix");
        return;
    }

    printf("%s (%zu x %zu):\n", label != NULL ? label : "Matrix",
           matrix->rows, matrix->cols);

    displayed_rows = matrix->rows > 6 ? 6 : matrix->rows;
    displayed_cols = matrix->cols > 6 ? 6 : matrix->cols;

    for (row_slot = 0; row_slot < displayed_rows; ++row_slot) {
        if (matrix->rows > 6 && row_slot == 3) {
            printf("[ ... ]\n");
        }

        if (matrix->rows > 6 && row_slot >= 3) {
            row = matrix->rows - (6 - row_slot);
        } else {
            row = row_slot;
        }

        printf("[");
        for (col_slot = 0; col_slot < displayed_cols; ++col_slot) {
            if (matrix->cols > 6 && col_slot == 3) {
                printf(" %8s", "...");
            }

            if (matrix->cols > 6 && col_slot >= 3) {
                col = matrix->cols - (6 - col_slot);
            } else {
                col = col_slot;
            }

            printf(" %8.3f", matrix->data[row * matrix->cols + col]);
        }
        printf(" ]\n");
    }
}
