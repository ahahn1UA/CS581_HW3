#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define DEAD 0
#define ALIVE 1

void initializeBoard(int **board, int size) {
    for (int i = 0; i < size + 2; i++) {
        for (int j = 0; j < size + 2; j++) {
            board[i][j] = DEAD;
        }
    }
    srand(52);

    for (int i = 1; i <= size; i++) {
        for (int j = 1; j <= size; j++) {
            board[i][j] = rand() % 2;
        }
    }
}

int countAliveNeighbors(int **board, int x, int y) {
    int count = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i != 0 || j != 0) {
                count += board[x + i][y + j];
            }
        }
    }
    return count;
}

void printBoardToFile(FILE *file, int **board, int size) {
    for (int i = 1; i <= size; i++) {
        for (int j = 1; j <= size; j++) {
            fprintf(file, board[i][j] == ALIVE ? "1 " : "0 ");
        }
        fprintf(file, "\n");
    }
    fprintf(file, "\n");
}

bool evolve(int **current, int **next, int size) {
    bool changed = false;
    for (int i = 1; i <= size; i++) {
        for (int j = 1; j <= size; j++) {
            int aliveNeighbors = countAliveNeighbors(current, i, j);
            if (current[i][j] == ALIVE) {
                next[i][j] = (aliveNeighbors == 2 || aliveNeighbors == 3) ? ALIVE : DEAD;
            } else {
                next[i][j] = (aliveNeighbors == 3) ? ALIVE : DEAD;
            }
            if (current[i][j] != next[i][j]) {
                changed = true;
            }
        }
    }
    return changed;
}

void swapBoards(int ***board1, int ***board2) {
    int **temp = *board1;
    *board1 = *board2;
    *board2 = temp;
}

int main(int argc, char *argv[]) {
    clock_t start, end;
    double cpu_time_used;
    start = clock();

    if (argc != 3) {
        printf("Usage: %s <size of board> <max generations>\n", argv[0]);
        return 1;
    }

    int size = atoi(argv[1]);
    int maxGenerations = atoi(argv[2]);

    int **board1 = (int **)malloc((size + 2) * sizeof(int *));
    int **board2 = (int **)malloc((size + 2) * sizeof(int *));
    for (int i = 0; i < size + 2; i++) {
        board1[i] = (int *)malloc((size + 2) * sizeof(int));
        board2[i] = (int *)malloc((size + 2) * sizeof(int));
    }

    initializeBoard(board1, size);
    initializeBoard(board2, size);

    int generation = 0;
    bool changed = true;

    FILE *outputFile = fopen("final_generation.txt", "w");
    if (outputFile == NULL) {
        printf("Error opening file!\n");
        return 1;
    }

    while (generation < maxGenerations && changed) {
        changed = evolve(board1, board2, size);
        swapBoards(&board1, &board2);
        generation++;
    }

    // Print final generation to file
    fprintf(outputFile, "Final Generation %d:\n", generation);
    printBoardToFile(outputFile, board1, size);
    fclose(outputFile);

    for (int i = 0; i < size + 2; i++) {
        free(board1[i]);
        free(board2[i]);
    }
    free(board1);
    free(board2);

    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Time taken: %f seconds\n", cpu_time_used);

    return 0;
}
