#include "matrix.h"

#include <windows.h>

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define multiplicationRepetitions 10
#define maxRepetitionsPerTimingBatch 10000000
#define minimumTimingSeconds 0.01
#define correctnessTolerance 1.0e-4f
#define inputBufferSize 128

typedef void (*MatrixMultiplyFunction)(
    const Matrix *a,
    const Matrix *b,
    Matrix *c
);

enum {
    sessionError = -1,
    sessionEof = 0,
    sessionComplete = 1
};

static int readInputLine(const char *prompt, char *buffer, size_t capacity)
{
    size_t length;
    int ch;

    fputs(prompt, stdout);
    fflush(stdout);

    if (fgets(buffer, (int)capacity, stdin) == NULL) {
        return 0;
    }

    length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[length - 1] = '\0';
        return 1;
    }

    if (!feof(stdin)) {
        while ((ch = getchar()) != '\n' && ch != EOF) {
        }
        return -1;
    }

    return 1;
}

static int readDimension(const char *prompt, size_t *value)
{
    char input[inputBufferSize];
    char *start;
    char *end;
    unsigned long long parsedValue;
    int inputStatus;

    for (;;) {
        inputStatus = readInputLine(prompt, input, sizeof(input));
        if (inputStatus == 0) {
            return 0;
        }
        if (inputStatus < 0) {
            puts("Input is too long. Enter one positive whole number.");
            continue;
        }

        start = input;
        while (isspace((unsigned char)*start)) {
            ++start;
        }

        if (*start == '-') {
            puts("Invalid dimension. Enter a positive whole number.");
            continue;
        }

        errno = 0;
        parsedValue = strtoull(start, &end, 10);
        if (start == end || errno == ERANGE) {
            puts("Invalid dimension. Enter a positive whole number.");
            continue;
        }

        while (isspace((unsigned char)*end)) {
            ++end;
        }

        if (*end != '\0' || parsedValue == 0 ||
            parsedValue > (unsigned long long)SIZE_MAX) {
            puts("Invalid dimension. Enter a positive whole number.");
            continue;
        }

        *value = (size_t)parsedValue;
        return 1;
    }
}

static int readCompatibleBRows(size_t aCols, size_t *bRows)
{
    for (;;) {
        if (!readDimension("ENTER MATRIX B ROWS: ", bRows)) {
            return 0;
        }

        if (*bRows == aCols) {
            return 1;
        }

        printf("Matrix B rows must equal Matrix A columns (%zu). "
               "Please re-enter Matrix B rows.\n", aCols);
    }
}

static int askRestart(int *restart)
{
    char input[inputBufferSize];
    char *choice;
    int inputStatus;

    for (;;) {
        inputStatus = readInputLine(
            "\nWould you like to re-enter dimensions and run again? "
            "(Y=restart, N=exit): ",
            input, sizeof(input));
        if (inputStatus == 0) {
            return 0;
        }
        if (inputStatus < 0) {
            puts("Please enter Y to restart or N to exit.");
            continue;
        }

        choice = input;
        while (isspace((unsigned char)*choice)) {
            ++choice;
        }

        if ((*choice == 'Y' || *choice == 'y') && choice[1] == '\0') {
            *restart = 1;
            return 1;
        }
        if ((*choice == 'N' || *choice == 'n') && choice[1] == '\0') {
            *restart = 0;
            return 1;
        }

        puts("Please enter Y to restart or N to exit.");
    }
}

static double averageMultiplicationTime(
    MatrixMultiplyFunction multiply,
    const Matrix *a,
    const Matrix *b,
    Matrix *c,
    const LARGE_INTEGER *frequency
)
{
    LARGE_INTEGER start;
    LARGE_INTEGER end;
    size_t batchRepetitions = multiplicationRepetitions;
    size_t repetitionCount = 0;
    size_t repetition;
    double elapsedSeconds;
    double estimatedTotalRepetitions;
    double estimatedRemainingRepetitions;

    if (frequency == NULL || frequency->QuadPart <= 0 ||
        !QueryPerformanceCounter(&start)) {
        return -1.0;
    }

    for (;;) {
        if (batchRepetitions > SIZE_MAX - repetitionCount) {
            return -1.0;
        }

        for (repetition = 0; repetition < batchRepetitions; ++repetition) {
            multiply(a, b, c);
        }
        repetitionCount += batchRepetitions;

        if (!QueryPerformanceCounter(&end)) {
            return -1.0;
        }

        elapsedSeconds = (double)(end.QuadPart - start.QuadPart) /
                          (double)frequency->QuadPart;
        if (elapsedSeconds >= minimumTimingSeconds) {
            break;
        }

        if (elapsedSeconds <= 0.0) {
            if (batchRepetitions >= maxRepetitionsPerTimingBatch) {
                return -1.0;
            }
            if (batchRepetitions > maxRepetitionsPerTimingBatch / 2) {
                batchRepetitions = maxRepetitionsPerTimingBatch;
            } else {
                batchRepetitions *= 2;
            }
        } else {
            estimatedTotalRepetitions =
                (double)repetitionCount * minimumTimingSeconds /
                elapsedSeconds;
            estimatedRemainingRepetitions =
                estimatedTotalRepetitions - (double)repetitionCount;

            if (estimatedRemainingRepetitions >=
                (double)maxRepetitionsPerTimingBatch) {
                batchRepetitions = maxRepetitionsPerTimingBatch;
            } else if (estimatedRemainingRepetitions <
                       (double)multiplicationRepetitions) {
                batchRepetitions = multiplicationRepetitions;
            } else {
                batchRepetitions =
                    (size_t)estimatedRemainingRepetitions + 1;
            }
        }
    }

    return elapsedSeconds / (double)repetitionCount;
}

static void printSpeedup(const char *baselineName,
                          double baselineSeconds,
                          double avx2Seconds)
{
    if (baselineSeconds > 0.0 && avx2Seconds > 0.0) {
        printf("AVX2 Speedup vs. %s: %.2fx\n",
               baselineName, baselineSeconds / avx2Seconds);
    } else {
        printf("AVX2 Speedup vs. %s: unavailable (timer resolution)\n",
               baselineName);
    }
}

static int runMatrixSession(void)
{
    Matrix a = {0};
    Matrix b = {0};
    Matrix cIjk = {0};
    Matrix cIkj = {0};
    Matrix cAvx2 = {0};
    size_t aRows;
    size_t aCols;
    size_t bRows;
    size_t bCols;
    double ijkSeconds;
    double ikjSeconds;
    double avx2Seconds;
    LARGE_INTEGER timerFrequency;
    int resultsMatch;
    int sessionStatus = sessionError;

    if (!QueryPerformanceFrequency(&timerFrequency) ||
        timerFrequency.QuadPart <= 0) {
        fprintf(stderr, "Unable to initialize the performance counter.\n");
        goto cleanup;
    }

    printf("\nGROUP 5's AVX2 MATRIX MULTIPLICATION PERFORMANCE\n\n");

    if (!readDimension("ENTER MATRIX A ROWS: ", &aRows) ||
        !readDimension("ENTER MATRIX A COLUMNS: ", &aCols) ||
        !readCompatibleBRows(aCols, &bRows) ||
        !readDimension("ENTER MATRIX B COLUMNS: ", &bCols)) {
        sessionStatus = sessionEof;
        goto cleanup;
    }

    if (!matrixCreate(&a, aRows, aCols) ||
        !matrixCreate(&b, bRows, bCols) ||
        !matrixCreate(&cIjk, aRows, bCols) ||
        !matrixCreate(&cIkj, aRows, bCols) ||
        !matrixCreate(&cAvx2, aRows, bCols)) {
        fprintf(stderr, "Unable to allocate the requested matrices.\n");
        goto cleanup;
    }

    matrixFillRandom(&a);
    matrixFillRandom(&b);

    printf("\nINPUT MATRICES\n\n");
    matrixPrint("Matrix A", &a);
    matrixPrint("Matrix B", &b);

    ijkSeconds = averageMultiplicationTime(
        matrixMultiplyIjk, &a, &b, &cIjk, &timerFrequency);
    ikjSeconds = averageMultiplicationTime(
        matrixMultiplyIkj, &a, &b, &cIkj, &timerFrequency);
    avx2Seconds = averageMultiplicationTime(
        matrixMultiplyAvx2, &a, &b, &cAvx2, &timerFrequency);

    if (ijkSeconds < 0.0 || ikjSeconds < 0.0 || avx2Seconds < 0.0) {
        fprintf(stderr, "The high-resolution performance counter failed.\n");
        goto cleanup;
    }

    printf("\nRESULTING MATRIX C (A x B)\n\n");
    matrixPrint("Matrix C", &cIjk);

    printf("\nPERFORMANCE COMPARISON\n\n");
    printf("Sequential i-j-k Execution Time: %.6f ms\n",
           ijkSeconds * 1000.0);
    printf("Sequential i-k-j Execution Time: %.6f ms\n",
           ikjSeconds * 1000.0);
    printf("AVX2 Assembly Execution Time:    %.6f ms\n",
           avx2Seconds * 1000.0);
    printSpeedup("Sequential i-j-k", ijkSeconds, avx2Seconds);
    printSpeedup("Sequential i-k-j", ikjSeconds, avx2Seconds);

    resultsMatch =
        matrixEqualTolerance(&cIjk, &cIkj, correctnessTolerance) &&
        matrixEqualTolerance(&cIjk, &cAvx2, correctnessTolerance) &&
        matrixEqualTolerance(&cIkj, &cAvx2, correctnessTolerance);

    printf("\nCORRECTNESS VERIFICATION\n\n");
    printf("Sequential i-j-k vs. Sequential i-k-j: %s\n",
           matrixEqualTolerance(&cIjk, &cIkj,
                                  correctnessTolerance) ? "PASSED" : "FAILED");
    printf("Sequential i-j-k vs. AVX2 Assembly:    %s\n",
           matrixEqualTolerance(&cIjk, &cAvx2,
                                  correctnessTolerance) ? "PASSED" : "FAILED");
    printf("Sequential i-k-j vs. AVX2 Assembly:    %s\n",
           matrixEqualTolerance(&cIkj, &cAvx2,
                                  correctnessTolerance) ? "PASSED" : "FAILED");

    if (resultsMatch) {
        printf("\nAll results match within the tolerance %.1e.\n",
               (double)correctnessTolerance);
        sessionStatus = sessionComplete;
    } else {
        fprintf(stderr, "\nOne or more matrix results do not match.\n");
    }

cleanup:
    matrixDestroy(&cAvx2);
    matrixDestroy(&cIkj);
    matrixDestroy(&cIjk);
    matrixDestroy(&b);
    matrixDestroy(&a);
    return sessionStatus;
}

int main(void)
{
    int restart = 1;
    int sessionStatus;
    int exitCode = 0;

    srand((unsigned int)time(NULL));

    while (restart) {
        sessionStatus = runMatrixSession();
        if (sessionStatus == sessionEof) {
            break;
        }
        if (sessionStatus == sessionError) {
            exitCode = 1;
        } else {
            exitCode = 0;
        }

        if (!askRestart(&restart)) {
            break;
        }
    }

    return exitCode;
}

