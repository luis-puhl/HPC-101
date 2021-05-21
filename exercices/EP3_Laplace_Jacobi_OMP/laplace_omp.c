/*
    This program solves Laplace's equation on a regular 2D grid using simple Jacobi iteration.
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define ITER_MAX 50000 // number of maximum iterations
#define CONV_THRESHOLD 1.0e-5f // threshold of convergence

int main(int argc, char *argv[]){
    if(argc != 3){
        printf("Usage: ./laplace_seq <N> <T>\n");
        printf("<N>: The size of each side of the domain (grid)\n");
        printf("<T>: The number of threads\n");
        exit(-1);
    }

    // size of each side of the grid
    int size = atoi(argv[1]);
    // matrix to be solved
    // allocate memory to the grid (matrix)
    double **grid = (double **) calloc(size, sizeof(double *));
    double **new_grid = (double **) calloc(size, sizeof(double *));
    for (int i = 0; i < size; i++){
        grid[i] = (double *) calloc(size, sizeof(double));
        new_grid[i] = (double *) calloc(size, sizeof(double));
    }

    // set grid initial conditions
    int bottom = size / 2;
    int top = bottom + size / 10;
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++){
            // inicializa região de calor no centro do grid
            if ( i>=bottom && i < top && j>=bottom && j<top) {
                grid[i][j] = 100;
            } else {
               grid[i][j] = 0;
            }
            new_grid[i][j] = 0.0;
        }
    }

    double err = 1.0;
    int iter = 0;

    fprintf(stderr, "%s Jacobi relaxation calculation: %d x %d grid\n", argv[0], size, size);

    // get the start time
    struct timeval time_start, time_end;
    gettimeofday(&time_start, NULL);

    // Jacobi iteration
    // This loop will end if either the maximum change reaches below a set threshold (convergence)
    // or a fixed number of maximum iterations have completed
    while ( err > CONV_THRESHOLD && iter <= ITER_MAX ) {

        err = 0.0;

        // calculates the Laplace equation to determine each cell's next value
        for( int i = 1; i < size-1; i++) {
            for(int j = 1; j < size-1; j++) {

                new_grid[i][j] = 0.25 * (grid[i][j+1] + grid[i][j-1] +
                                         grid[i-1][j] + grid[i+1][j]);

                double diff = new_grid[i][j] - grid[i][j];
                if (diff < 0) {
                    diff = -diff;
                }
                if (diff > err) {
                    err = diff;
                }
            }
        }

        // copie the next values into the working array for the next iteration
        for( int i = 1; i < size-1; i++) {
            for( int j = 1; j < size-1; j++) {
                grid[i][j] = new_grid[i][j];
            }
        }

        if(iter % 1000 == 0) {
            fprintf(stderr, "Error of %le at iteration %d\n", err, iter);
        }

        if(iter % 500 == 0) {
            for(int i = 0; i < size; i++){
                for(int j = 0; j < size; j++){
                    printf("%lf ", grid[i][j]);
                }
                printf("\n");
            }
            printf("\n");
        }

        iter++;
    }

    // get the end time
    gettimeofday(&time_end, NULL);
    double exec_time = (double) (time_end.tv_sec - time_start.tv_sec) +
                       (double) (time_end.tv_usec - time_start.tv_usec) / 1000000.0;

    fprintf(stderr, "\n%s Kernel executed in %lf seconds with %d iterations and error of %le\n", argv[0], exec_time, iter, err);

    return 0;
}
