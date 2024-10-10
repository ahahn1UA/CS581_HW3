#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <time.h>

// gcc -fopenmp -o HW3 HW3.c
// icx -fopenmp -O -o HW1.out HW1.c
// ./HW3 100 1000 4 ./output

int main(int argc, char* argv[]) {
    // Check for correct number of command-line arguments
    if (argc != 5) {
        fprintf(stderr, "Usage: %s problem_size max_iterations num_threads output_directory\n", argv[0]);
        return 1;
    }

    // Parse command-line arguments
    int N = atoi(argv[1]);                 // Problem size
    int max_iterations = atoi(argv[2]);    // Maximum number of iterations
    int num_threads = atoi(argv[3]);       // Number of threads
    char* output_directory = argv[4];      // Output directory

    // Validate the input arguments
    if (N <= 0 || max_iterations <= 0 || num_threads <= 0) {
        fprintf(stderr, "Error: problem_size, max_iterations, and num_threads must be positive integers.\n");
        return 1;
    }

    // Set the number of threads for OpenMP
    omp_set_num_threads(num_threads);

    // Calculate the total size of the grid including ghost cells
    int grid_size = N + 2; // Including ghost cells

    // Dynamically allocate memory for the current and next grids
    int** current_grid = (int**)malloc(grid_size * sizeof(int*));
    int** next_grid = (int**)malloc(grid_size * sizeof(int*));

    // Check if memory allocation was successful
    if (current_grid == NULL || next_grid == NULL) {
        fprintf(stderr, "Error: Unable to allocate memory for grid rows.\n");
        return 1;
    }

    // Allocate each row
    for (int i = 0; i < grid_size; i++) {
        current_grid[i] = (int*)malloc(grid_size * sizeof(int));
        next_grid[i] = (int*)malloc(grid_size * sizeof(int));
        if (current_grid[i] == NULL || next_grid[i] == NULL) {
            fprintf(stderr, "Error: Unable to allocate memory for grid columns.\n");
            return 1;
        }
    }

    // Seed the random number generator
    srand(52);

    // Initialize the grids
    // Set all cells to 0
    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            current_grid[i][j] = 0;
            next_grid[i][j] = 0;
        }
    }

    // square
    // current_grid[2][2] = 1;
    // current_grid[2][3] = 1;
    // current_grid[3][2] = 1;
    // current_grid[3][3] = 1;

    // blinker
    // current_grid[2][2] = 1;
    // current_grid[2][3] = 1;
    // current_grid[2][4] = 1;

    // Initialize the inner cells (excluding ghost cells) randomly to 0 or 1
    for (int i = 1; i <= N; i++) {
        for (int j = 1; j <= N; j++) {
            current_grid[i][j] = rand() % 2;
        }
    }

    // Start the timer
    double start_time = omp_get_wtime();

    // Variable to check for convergence
    int converged = 0;
    int changed = 0;
    int iterations_completed = 0; // Variable to track the number of iterations completed

    // Main iteration loop
    #pragma omp parallel shared(current_grid, next_grid, N, max_iterations, converged, changed, iterations_completed)
    {
        int iteration;
        for (iteration = 1; iteration <= max_iterations && !converged; iteration++) {
            // Reset the changed counter at the beginning of each iteration
            #pragma omp single
            {
                changed = 0;
            }

            // Compute the new state of the grid
            #pragma omp for reduction(+:changed)
            for (int i = 1; i <= N; i++) {
                for (int j = 1; j <= N; j++) {
                    int num_neighbors = 0;

                    // Calculate the number of live neighbors
                    num_neighbors += current_grid[i - 1][j - 1];
                    num_neighbors += current_grid[i - 1][j];
                    num_neighbors += current_grid[i - 1][j + 1];
                    num_neighbors += current_grid[i][j - 1];
                    num_neighbors += current_grid[i][j + 1];
                    num_neighbors += current_grid[i + 1][j - 1];
                    num_neighbors += current_grid[i + 1][j];
                    num_neighbors += current_grid[i + 1][j + 1];

                    // Apply the Game of Life rules
                    int new_state = current_grid[i][j];
                    if (current_grid[i][j] == 1) {
                        // Live cell rules
                        if (num_neighbors < 2 || num_neighbors > 3) {
                            new_state = 0; // Cell dies
                        }
                    } else {
                        // Dead cell rules
                        if (num_neighbors == 3) {
                            new_state = 1; // Cell becomes alive
                        }
                    }

                    // Check if the cell state has changed
                    if (new_state != current_grid[i][j]) {
                        changed++;
                    }

                    // Update the next grid
                    next_grid[i][j] = new_state;
                }
            }

            // Synchronize threads before proceeding
            #pragma omp barrier

            // Swap the grids and check for convergence
            #pragma omp single
            {
                // Swap the current and next grids
                int** temp = current_grid;
                current_grid = next_grid;
                next_grid = temp;

                // If no cells have changed, set the convergence flag
                if (changed == 0) {
                    printf("No change detected at iteration %d\n", iteration);
                    converged = 1;
                }

                // Update the number of iterations completed
                iterations_completed = iteration;
            }

            // Synchronize threads before the next iteration
            #pragma omp barrier
        }
    }

    // Stop the timer
    double end_time = omp_get_wtime();
    double total_time = end_time - start_time;

    // Print the total execution time and the number of iterations completed
    printf("Total iterations completed: %d\n", iterations_completed);
    printf("Total time: %f seconds\n", total_time);

    // Construct the output file path
    char output_file_path[256];
    snprintf(output_file_path, sizeof(output_file_path), "%s.txt", output_directory);

    // Open the output file for writing
    FILE* output_file = fopen(output_file_path, "w");
    if (output_file == NULL) {
        fprintf(stderr, "Error: Unable to open output file.\n");
        return 1;
    }

    // Write the final state of the grid to the output file
    for (int i = 1; i <= N; i++) {
        for (int j = 1; j <= N; j++) {
            fprintf(output_file, "%d ", current_grid[i][j]);
        }
        fprintf(output_file, "\n");
    }

    // Close the output file
    fclose(output_file);

    // Free the allocated memory
    for (int i = 0; i < grid_size; i++) {
        free(current_grid[i]);
        free(next_grid[i]);
    }
    free(current_grid);
    free(next_grid);

    return 0;
}
