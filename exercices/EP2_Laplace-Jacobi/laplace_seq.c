/*
    This program solves Laplace's equation on a regular 2D grid using simple Jacobi iteration.
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#define ITER_MAX 50000 // number of maximum iterations
#define CONV_THRESHOLD 1.0e-8f // threshold of convergence

int main(int argc, char *argv[]){
    // seed for random generator
    srand(10);
    if(argc != 2){
        printf("Usage: ./laplace_seq N\n");
        printf("N: The size of each side of the domain (grid)\n");
        exit(-1);
    }

    // variables to measure execution time
    struct timeval time_start;
    struct timeval time_end;

    // size of each side of the grid
    int size = atoi(argv[1]);

    // allocate memory to the grid (matrix)
    // matrix to be solved
    double **grid = (double **)calloc(size, sizeof(double *));

    // auxiliary matrix
    double **new_grid = (double **)calloc(size, sizeof(double *));
    double **swap_grid;

    for(int i = 0; i < size; i++){
        grid[i] = (double *) calloc(size, sizeof(double));
        new_grid[i] = (double *) calloc(size, sizeof(double));
    }

    // set grid initial conditions
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++){
            grid[i][j] = rand() % 100;
            new_grid[i][j] = grid[i][j];
            printf("%lf ", grid[i][j]);
        }
        printf("\n");
    }
    printf("Jacobi relaxation calculation: %d x %d grid\t%s\n", size, size, argv[0]);

    // get the start time
    gettimeofday(&time_start, NULL);

    // Jacobi iteration
    // This loop will end if either the maximum change reaches below a set threshold (convergence)
    // or a fixed number of maximum iterations have completed
    double err = 0.0;
    int iter;
    for (iter = 0; iter <= ITER_MAX; iter++) {
        err = 0.0;

        // calculates the Laplace equation to determine each cell's next value
        for( int i = 1; i < size-1; i++) {
            for(int j = 1; j < size-1; j++) {

                new_grid[i][j] = 0.25 * (grid[i][j+1] + grid[i][j-1] +
                                         grid[i-1][j] + grid[i+1][j]);

                // err = max(err, absolute(new_grid[i][j] - grid[i][j]));
                double diff = new_grid[i][j] - grid[i][j];
                // abs
                if (diff < 0) {
                    diff *= -1.0;
                }
                // max
                // if (diff > err) {
                //     err = diff;
                // }
                err += diff;
            }
        }
        if (err < CONV_THRESHOLD) {
            fprintf(stderr, "main break iteration=%d, err=%le\n", iter, err);
            break;
        }

        swap_grid = grid;
        grid = new_grid;
        new_grid = swap_grid;
        
        if (iter < 3 || (iter % (ITER_MAX / 100)) == 0) {
            fprintf(stderr, "iteration=%d, err=%le\n", iter, err);
            // fprintf(stderr, "Root, grid(%p, %p)\n", grids.now, grids.next);
        }
    }

    // get the end time
    gettimeofday(&time_end, NULL);

    double exec_time = (double) (time_end.tv_sec - time_start.tv_sec) +
                       (double) (time_end.tv_usec - time_start.tv_usec) / 1000000.0;

    //save the final grid in file
    for(int i = 0; i < size; i++){
        for(int j = 0; j < size; j++){
            printf("%lf ", grid[i][j]);
        }
        printf("\n");
    }

    printf("\n%s Kernel executed in %le seconds with %d iterations and error of %le\n", argv[0], exec_time, iter, err);

    return 0;
}
