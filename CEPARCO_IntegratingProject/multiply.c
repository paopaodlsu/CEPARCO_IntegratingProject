#include "matrix.h"

#include <math.h>

static int multiplicationArgumentsAreValid(
    const Matrix *a,
    const Matrix *b,
    const Matrix *c
)
{
    return a != NULL && b != NULL && c != NULL &&
           a->data != NULL && b->data != NULL && c->data != NULL &&
           a->cols == b->rows &&
           c->rows == a->rows &&
           c->cols == b->cols;
}

void matrixMultiplyIjk(const Matrix *a, const Matrix *b, Matrix *c)
{
    size_t i;
    size_t j;
    size_t k;

    if (!multiplicationArgumentsAreValid(a, b, c)) {
        return;
    }

    for (i = 0; i < a->rows; ++i) {
        for (j = 0; j < b->cols; ++j) {
            float sum = 0.0f;
            for (k = 0; k < a->cols; ++k) {
                sum += a->data[i * a->cols + k] *
                       b->data[k * b->cols + j];
            }
            c->data[i * c->cols + j] = sum;
        }
    }
}

void matrixMultiplyIkj(const Matrix *a, const Matrix *b, Matrix *c)
{
    size_t i;
    size_t j;
    size_t k;

    if (!multiplicationArgumentsAreValid(a, b, c)) {
        return;
    }

    for (i = 0; i < c->rows; ++i) {
        for (j = 0; j < c->cols; ++j) {
            c->data[i * c->cols + j] = 0.0f;
        }
    }

    for (i = 0; i < a->rows; ++i) {
        for (k = 0; k < a->cols; ++k) {
            float aValue = a->data[i * a->cols + k];
            for (j = 0; j < b->cols; ++j) {
                c->data[i * c->cols + j] +=
                    aValue * b->data[k * b->cols + j];
            }
        }
    }
}

int matrixEqualTolerance(
    const Matrix *left,
    const Matrix *right,
    float tolerance
)
{
    size_t i;
    size_t elementCount;

    if (left == NULL || right == NULL ||
        left->data == NULL || right->data == NULL ||
        left->rows != right->rows || left->cols != right->cols ||
        tolerance < 0.0f) {
        return 0;
    }

    elementCount = left->rows * left->cols;
    for (i = 0; i < elementCount; ++i) {
        if (!(fabsf(left->data[i] - right->data[i]) <= tolerance)) {
            return 0;
        }
    }

    return 1;
}
