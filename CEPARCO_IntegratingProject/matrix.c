#include "matrix.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int matrixCreate(Matrix *matrix, size_t rows, size_t cols)
{
    size_t elementCount;
    float *data;

    if (matrix == NULL || rows == 0 || cols == 0) {
        return 0;
    }

    if (rows > SIZE_MAX / cols) {
        return 0;
    }

    elementCount = rows * cols;
    if (elementCount > SIZE_MAX / sizeof(*data)) {
        return 0;
    }

    data = (float *)malloc(elementCount * sizeof(*data));
    if (data == NULL) {
        return 0;
    }

    matrix->rows = rows;
    matrix->cols = cols;
    matrix->data = data;
    return 1;
}

void matrixDestroy(Matrix *matrix)
{
    if (matrix == NULL) {
        return;
    }

    free(matrix->data);
    matrix->rows = 0;
    matrix->cols = 0;
    matrix->data = NULL;
}

void matrixFillRandom(Matrix *matrix)
{
    size_t i;
    size_t elementCount;

    if (matrix == NULL || matrix->data == NULL) {
        return;
    }

    elementCount = matrix->rows * matrix->cols;
    for (i = 0; i < elementCount; ++i) {
        matrix->data[i] = (float)rand() / (float)RAND_MAX;
    }
}

void matrixPrint(const char *label, const Matrix *matrix)
{
    size_t rowSlot;
    size_t colSlot;
    size_t row;
    size_t col;
    size_t displayedRows;
    size_t displayedCols;

    if (matrix == NULL || matrix->data == NULL) {
        printf("%s: unavailable\n", label != NULL ? label : "Matrix");
        return;
    }

    printf("%s (%zu x %zu):\n", label != NULL ? label : "Matrix",
           matrix->rows, matrix->cols);

    displayedRows = matrix->rows > 6 ? 6 : matrix->rows;
    displayedCols = matrix->cols > 6 ? 6 : matrix->cols;

    for (rowSlot = 0; rowSlot < displayedRows; ++rowSlot) {
        if (matrix->rows > 6 && rowSlot == 3) {
            printf("[ ... ]\n");
        }

        if (matrix->rows > 6 && rowSlot >= 3) {
            row = matrix->rows - (6 - rowSlot);
        } else {
            row = rowSlot;
        }

        printf("[");
        for (colSlot = 0; colSlot < displayedCols; ++colSlot) {
            if (matrix->cols > 6 && colSlot == 3) {
                printf(" %8s", "...");
            }

            if (matrix->cols > 6 && colSlot >= 3) {
                col = matrix->cols - (6 - colSlot);
            } else {
                col = colSlot;
            }

            printf(" %8.3f", matrix->data[row * matrix->cols + col]);
        }
        printf(" ]\n");
    }
}
