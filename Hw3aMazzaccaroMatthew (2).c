#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define GRID_SIZE 9

typedef int bool;
#define TRUE 1
#define FALSE 0

int sudokuPuzzle[GRID_SIZE][GRID_SIZE]; // Shared Sudoku puzzle
bool columnValid[GRID_SIZE]; // Shared boolean array for columns
bool rowValid[GRID_SIZE]; // Shared boolean array for rows
bool subgridValid[GRID_SIZE]; // Shared boolean array for subgrids

typedef struct {
    int topRow;
    int bottomRow;
    int leftColumn;
    int rightColumn;
} IndexRange;

void *checkColumn(void *param);
void *checkRow(void *param);
void *checkSubgrid(void *param);

void readSudokuPuzzle() {
    FILE *fp = fopen("SudokuPuzzle.txt", "r");
    if (fp == NULL) {
        printf("Error opening file.\n");
        return;
    }

    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE-1; j++) {
            fscanf(fp, "%d\t", &sudokuPuzzle[i][j]);
        }
        fscanf(fp, "%d\n", &sudokuPuzzle[i][8]);
    }

    fclose(fp);
}

void validateSudokuPuzzle() {
    // Initialize thread IDs
    pthread_t tid_column[GRID_SIZE];
    pthread_t tid_row[GRID_SIZE];
    pthread_t tid_subgrid[GRID_SIZE];

    // Create worker threads for columns
    IndexRange columnRanges[GRID_SIZE];
    for (int i = 0; i < GRID_SIZE; i++) {
        columnRanges[i].topRow = 0;
        columnRanges[i].bottomRow = GRID_SIZE - 1;
        columnRanges[i].leftColumn = i;
        columnRanges[i].rightColumn = i;

        pthread_create(&(tid_column[i]), NULL, checkColumn, &(columnRanges[i]));
    }

    // Create worker threads for rows
    IndexRange rowRanges[GRID_SIZE];
    for (int i = 0; i < GRID_SIZE; i++) {
        rowRanges[i].topRow = i;
        rowRanges[i].bottomRow = i;
        rowRanges[i].leftColumn = 0;
        rowRanges[i].rightColumn = GRID_SIZE - 1;

        pthread_create(&(tid_row[i]), NULL, checkRow, &(rowRanges[i]));
    }

    // Create worker threads for subgrids
    IndexRange subgridRanges[GRID_SIZE];
    int subgridIndex = 0;
    for (int i = 0; i < GRID_SIZE; i += 3) {
        for (int j = 0; j < GRID_SIZE; j += 3) {
            subgridRanges[subgridIndex].topRow = i;
            subgridRanges[subgridIndex].bottomRow = i + 2;
            subgridRanges[subgridIndex].leftColumn = j;
            subgridRanges[subgridIndex].rightColumn = j + 2;

            pthread_create(&(tid_subgrid[subgridIndex]), NULL, checkSubgrid, &(subgridRanges[subgridIndex]));

            subgridIndex++;
        }
    }

    // Wait for all worker threads to complete
    for (int i = 0; i < GRID_SIZE; i++) {
        pthread_join(tid_column[i], NULL);
        pthread_join(tid_row[i], NULL);
        pthread_join(tid_subgrid[i], NULL);
    }

    // Print column results
    printf("Column Results:\n");
    for (int i = 0; i < GRID_SIZE; i++) {
        printf("%x: %s\n", i, columnValid[i] ? "Valid" : "Invalid");
    }
    printf("\n");

    // Print row results
    printf("Row Results:\n");
    for (int i = 0; i < GRID_SIZE; i++) {
        printf("%x: %s\n", i, rowValid[i] ? "Valid" : "Invalid");
    }
    printf("\n");

    // Print subgrid results
    printf("Subgrid Results:\n");
    for (int i = 0; i < GRID_SIZE; i++) {
        printf("%x: %s\n", i, subgridValid[i] ? "Valid" : "Invalid");
    }

    // Check puzzle validity
    bool puzzleValid = TRUE;
    for (int i = 0; i < GRID_SIZE; i++) {
        if (!columnValid[i] || !rowValid[i] || !subgridValid[i]) {
            puzzleValid = FALSE;
            break;
        }
    }

    printf("\nSudoku Puzzle: %s\n", puzzleValid ? "Valid" : "Invalid");
}

void *checkColumn(void *param) {
    IndexRange *range = (IndexRange *)param;
    int leftColumn = range->leftColumn;
    int rightColumn = range->rightColumn;

    for (int col = leftColumn; col <= rightColumn; col++) {
        bool used[GRID_SIZE] = { FALSE };

        for (int row = range->topRow; row <= range->bottomRow; row++) {
            int num = sudokuPuzzle[row][col];

            if (num < 1 || num > GRID_SIZE || used[num - 1]) {
                printf("%x TRow: %d, BRow: %d, LCol: %d, RCol: %d invalid!\n", (int)pthread_self(), range->topRow, range->bottomRow, leftColumn, rightColumn);
                columnValid[col] = FALSE;
                pthread_exit(NULL);
            }

            used[num - 1] = TRUE;
        }
    }

    printf("%x TRow: %d, BRow: %d, LCol: %d, RCol: %d valid!\n", (int)pthread_self(), range->topRow, range->bottomRow, leftColumn, rightColumn);
    columnValid[leftColumn] = TRUE;
    pthread_exit(NULL);
}

void *checkRow(void *param) {
    IndexRange *range = (IndexRange *)param;
    int topRow = range->topRow;
    int bottomRow = range->bottomRow;

    for (int row = topRow; row <= bottomRow; row++) {
        bool used[GRID_SIZE] = { FALSE };

        for (int col = range->leftColumn; col <= range->rightColumn; col++) {
            int num = sudokuPuzzle[row][col];

            if (num < 1 || num > GRID_SIZE || used[num - 1]) {
                printf("%x TRow: %d, BRow: %d, LCol: %d, RCol: %d invalid!\n", (int)pthread_self(), topRow, bottomRow, range->leftColumn, range->rightColumn);
                rowValid[row] = FALSE;
                pthread_exit(NULL);
            }

            used[num - 1] = TRUE;
        }
    }

    printf("%x TRow: %d, BRow: %d, LCol: %d, RCol: %d valid!\n", (int)pthread_self(), topRow, bottomRow, range->leftColumn, range->rightColumn);
    rowValid[topRow] = TRUE;
    pthread_exit(NULL);
}

void *checkSubgrid(void *param) {
    IndexRange *range = (IndexRange *)param;

    bool used[GRID_SIZE] = { FALSE };

    for (int row = range->topRow; row <= range->bottomRow; row++) {
        for (int col = range->leftColumn; col <= range->rightColumn; col++) {
            int num = sudokuPuzzle[row][col];

            if (num < 1 || num > GRID_SIZE || used[num - 1]) {
                printf("%x TRow: %d, BRow: %d, LCol: %d, RCol: %d invalid!\n", (int)pthread_self(), range->topRow, range->bottomRow, range->leftColumn, range->rightColumn);
                int subgridIndex = (range->topRow / 3) * 3 + (range->leftColumn / 3);
                subgridValid[subgridIndex] = FALSE;
                pthread_exit(NULL);
            }

            used[num - 1] = TRUE;
        }
    }

    printf("%x TRow: %d, BRow: %d, LCol: %d, RCol: %d valid!\n", (int)pthread_self(), range->topRow, range->bottomRow, range->leftColumn, range->rightColumn);
    int subgridIndex = (range->topRow / 3) * 3 + (range->leftColumn / 3);
    subgridValid[subgridIndex] = TRUE;
    pthread_exit(NULL);
}

int main() {
    readSudokuPuzzle();
    validateSudokuPuzzle();

    return 0;
}
