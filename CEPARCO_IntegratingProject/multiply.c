#include "matrix.h"

#include <math.h>

static int multiplication_arguments_are_valid(
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

void matrix_multiply_ijk(const Matrix *a, const Matrix *b, Matrix *c)
{
    size_t i;
    size_t j;
    size_t k;

    if (!multiplication_arguments_are_valid(a, b, c)) {
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

void matrix_multiply_ikj(const Matrix *a, const Matrix *b, Matrix *c)
{
    size_t i;
    size_t j;
    size_t k;

    if (!multiplication_arguments_are_valid(a, b, c)) {
        return;
    }

    for (i = 0; i < c->rows; ++i) {
        for (j = 0; j < c->cols; ++j) {
            c->data[i * c->cols + j] = 0.0f;
        }
    }

    for (i = 0; i < a->rows; ++i) {
        for (k = 0; k < a->cols; ++k) {
            float a_value = a->data[i * a->cols + k];
            for (j = 0; j < b->cols; ++j) {
                c->data[i * c->cols + j] +=
                    a_value * b->data[k * b->cols + j];
            }
        }
    }
}

int matrix_equal_tolerance(
    const Matrix *left,
    const Matrix *right,
    float tolerance
)
{
    size_t i;
    size_t element_count;

    if (left == NULL || right == NULL ||
        left->data == NULL || right->data == NULL ||
        left->rows != right->rows || left->cols != right->cols ||
        tolerance < 0.0f) {
        return 0;
    }

    element_count = left->rows * left->cols;
    for (i = 0; i < element_count; ++i) {
        if (!(fabsf(left->data[i] - right->data[i]) <= tolerance)) {
            return 0;
        }
    }

    return 1;
}
