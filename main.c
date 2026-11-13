#include "matrix.h"

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MULTIPLICATION_REPETITIONS 10
#define MINIMUM_TIMING_SECONDS 0.01
#define CORRECTNESS_TOLERANCE 1.0e-4f

typedef void (*MatrixMultiplyFunction)(
    const Matrix* a,
    const Matrix* b,
    Matrix* c
    );

static int read_dimension(const char* prompt, size_t* value)
{
    char line[128];

    for (;;) {
        char* p = line;
        char* end;
        unsigned long long parsed = 0;
        int valid = 1;

        printf("%s", prompt);
        fflush(stdout);

        if (fgets(line, sizeof line, stdin) == NULL) {
            fprintf(stderr, "\nNo input received.\n");
            return 0;
        }

        if (strchr(line, '\n') == NULL && !feof(stdin)) {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {
            }
            valid = 0;
        }

        if (valid) {
            while (isspace((unsigned char)*p)) {
                ++p;
            }

            if (!isdigit((unsigned char)*p)) {
                valid = 0;
            }
            else {
                errno = 0;
                parsed = strtoull(p, &end, 10);
                if (errno == ERANGE ||
                    parsed == 0 ||
                    parsed > (unsigned long long)SIZE_MAX) {
                    valid = 0;
                }
                else {
                    while (isspace((unsigned char)*end)) {
                        ++end;
                    }
                    if (*end != '\0') {
                        valid = 0;
                    }
                }
            }
        }

        if (valid) {
            *value = (size_t)parsed;
            return 1;
        }

        fprintf(stderr,
            "Invalid dimension. Enter a positive whole number "
            "(no negatives, letters, or symbols).\n");
    }
}

static double average_multiplication_time(
    MatrixMultiplyFunction multiply,
    const Matrix* a,
    const Matrix* b,
    Matrix* c
)
{
    clock_t start;
    clock_t end;
    size_t repetition_count = 0;
    int repetition;

    start = clock();
    if (start == (clock_t)-1) {
        return -1.0;
    }

    do {
        for (repetition = 0;
            repetition < MULTIPLICATION_REPETITIONS;
            ++repetition) {
            multiply(a, b, c);
        }

        repetition_count += MULTIPLICATION_REPETITIONS;
        end = clock();
        if (end == (clock_t)-1) {
            return -1.0;
        }
    } while ((double)(end - start) / (double)CLOCKS_PER_SEC <
        MINIMUM_TIMING_SECONDS);

    return (double)(end - start) /
        (double)CLOCKS_PER_SEC /
        (double)repetition_count;
}

static void print_speedup(const char* baseline_name,
    double baseline_seconds,
    double avx2_seconds)
{
    if (baseline_seconds > 0.0 && avx2_seconds > 0.0) {
        printf("AVX2 Speedup vs. %s: %.2fx\n",
            baseline_name, baseline_seconds / avx2_seconds);
    }
    else {
        printf("AVX2 Speedup vs. %s: unavailable (timer resolution)\n",
            baseline_name);
    }
}

int main(void)
{
    Matrix a = { 0 };
    Matrix b = { 0 };
    Matrix c_ijk = { 0 };
    Matrix c_ikj = { 0 };
    Matrix c_avx2 = { 0 };
    size_t a_rows;
    size_t a_cols;
    size_t b_rows;
    size_t b_cols;
    double ijk_seconds;
    double ikj_seconds;
    double avx2_seconds;
    int results_match;
    int exit_code = 1;

    printf("AVX2 MATRIX MULTIPLICATION PERFORMANCE\n\n");

    if (!read_dimension("ENTER MATRIX A ROWS: ", &a_rows) ||
        !read_dimension("ENTER MATRIX A COLUMNS: ", &a_cols) ||
        !read_dimension("ENTER MATRIX B ROWS: ", &b_rows) ||
        !read_dimension("ENTER MATRIX B COLUMNS: ", &b_cols)) {
        goto cleanup;
    }

    if (a_cols != b_rows) {
        fprintf(stderr,
            "Incompatible dimensions: Matrix A columns must equal "
            "Matrix B rows.\n");
        goto cleanup;
    }

    if (!matrix_create(&a, a_rows, a_cols) ||
        !matrix_create(&b, b_rows, b_cols) ||
        !matrix_create(&c_ijk, a_rows, b_cols) ||
        !matrix_create(&c_ikj, a_rows, b_cols) ||
        !matrix_create(&c_avx2, a_rows, b_cols)) {
        fprintf(stderr, "Unable to allocate the requested matrices.\n");
        goto cleanup;
    }

    srand((unsigned int)time(NULL));
    matrix_fill_random(&a);
    matrix_fill_random(&b);

    printf("\nINPUT MATRICES\n\n");
    matrix_print("Matrix A", &a);
    matrix_print("Matrix B", &b);

    ijk_seconds = average_multiplication_time(
        matrix_multiply_ijk, &a, &b, &c_ijk);
    ikj_seconds = average_multiplication_time(
        matrix_multiply_ikj, &a, &b, &c_ikj);
    avx2_seconds = average_multiplication_time(
        matrix_multiply_avx2, &a, &b, &c_avx2);

    if (ijk_seconds < 0.0 || ikj_seconds < 0.0 || avx2_seconds < 0.0) {
        fprintf(stderr, "The C clock() function failed.\n");
        goto cleanup;
    }

    printf("\nRESULTING MATRIX C (A x B)\n\n");
    matrix_print("Matrix C", &c_ijk);

    printf("\nPERFORMANCE COMPARISON\n\n");
    printf("Sequential i-j-k Execution Time: %.6f ms\n",
        ijk_seconds * 1000.0);
    printf("Sequential i-k-j Execution Time: %.6f ms\n",
        ikj_seconds * 1000.0);
    printf("AVX2 Assembly Execution Time:    %.6f ms\n",
        avx2_seconds * 1000.0);
    print_speedup("Sequential i-j-k", ijk_seconds, avx2_seconds);
    print_speedup("Sequential i-k-j", ikj_seconds, avx2_seconds);

    results_match =
        matrix_equal_tolerance(&c_ijk, &c_ikj, CORRECTNESS_TOLERANCE) &&
        matrix_equal_tolerance(&c_ijk, &c_avx2, CORRECTNESS_TOLERANCE) &&
        matrix_equal_tolerance(&c_ikj, &c_avx2, CORRECTNESS_TOLERANCE);

    printf("\nCORRECTNESS VERIFICATION\n\n");
    printf("Sequential i-j-k vs. Sequential i-k-j: %s\n",
        matrix_equal_tolerance(&c_ijk, &c_ikj,
            CORRECTNESS_TOLERANCE) ? "PASSED" : "FAILED");
    printf("Sequential i-j-k vs. AVX2 Assembly:    %s\n",
        matrix_equal_tolerance(&c_ijk, &c_avx2,
            CORRECTNESS_TOLERANCE) ? "PASSED" : "FAILED");
    printf("Sequential i-k-j vs. AVX2 Assembly:    %s\n",
        matrix_equal_tolerance(&c_ikj, &c_avx2,
            CORRECTNESS_TOLERANCE) ? "PASSED" : "FAILED");

    if (results_match) {
        printf("\nAll results match within the tolerance %.1e.\n",
            (double)CORRECTNESS_TOLERANCE);
        exit_code = 0;
    }
    else {
        fprintf(stderr, "\nOne or more matrix results do not match.\n");
    }

cleanup:
    matrix_destroy(&c_avx2);
    matrix_destroy(&c_ikj);
    matrix_destroy(&c_ijk);
    matrix_destroy(&b);
    matrix_destroy(&a);
    return exit_code;
}