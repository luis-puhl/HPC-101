/*
    This program solves Laplace's equation on a regular 2D grid using simple Jacobi iteration.
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>

#define ITER_MAX 50000 // number of maximum iterations
#define CONV_THRESHOLD 1.0e-5f // threshold of convergence

int main(int argc, char *argv[]){
    if(argc != 3){
        printf("Usage: ./laplace_seq <N> <T>\n");
        printf("<N>: The size of each side of the domain (grid)\n");
        printf("<T>: The number of threads\n");
        exit(-1);
    }
    int threads;
    omp_set_num_threads(atoi(argv[2]));

    // size of each side of the grid
    int size = atoi(argv[1]);
    // matrix to be solved
    // allocate memory to the grid (matrix)
    // double *grid = (double *) calloc(size * size, sizeof(double));
    // double *new_grid = (double *) calloc(size * size, sizeof(double));
    double **grid = (double **) calloc(size, sizeof(double *));
    double **new_grid = (double **) calloc(size, sizeof(double *));
    // double *thread_grids[threads];
    // #pragma omp parallel for
    // for (int i = 0; i < threads; i++){
    //     thread_grids[i] = (double *) calloc(size * size, sizeof(double));
    // }
    // double **thread_grids[threads];
    // #pragma omp parallel for
    // for (int t = 0; t < threads; t++){
    //     thread_grids[t] = (double **) calloc(size, sizeof(double*));
    // }
    #pragma omp parallel for
    for (int i = 0; i < size; i++){
        grid[i] = (double *) calloc(size, sizeof(double));  
        new_grid[i] = (double *) calloc(size, sizeof(double));
        // for (int t = 0; t < threads; t++){
        //     thread_grids[t][i] = (double *) calloc(size, sizeof(double));
        // }
    }

    // set grid initial conditions
    int bottom = size / 2;
    int top = bottom + size / 10;
    if (top == bottom) {
        top++;
    }
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++){
            // inicializa região de calor no centro do grid
            if ( i>=bottom && i < top && j>=bottom && j<top) {
                grid[i][j] = 100;
            } else {
               grid[i][j] = 0;
            }
            new_grid[i][j] = 0.0;
            // for (int t = 0; t < threads; t++){
            //     thread_grids[t][i][j] = grid[i][j];
            // }
        }
    }

    fprintf(stderr, "%s Jacobi relaxation calculation: %d x %d grid\n", argv[0], size, size);
    for(int i = 0; i < size; i++){
        for(int j = 0; j < size; j++){
            printf("%lf ", grid[i][j]);
        }
        printf("\n");
    }
    printf("\n");

    // get the start time
    struct timeval time_start, time_end;
    gettimeofday(&time_start, NULL);

    // Jacobi iteration
    // This loop will end if either the maximum change reaches below a set threshold (convergence)
    // or a fixed number of maximum iterations have completed
    double err = 1.0;
    int iter = 0;
    while ( err > CONV_THRESHOLD && iter <= ITER_MAX ) {
        err = 0.0;
        // calculates the Laplace equation to determine each cell's next value
        int i, j;
        #pragma omp parallel for collapse(2) reduction(max: err) private(i,j) schedule(static)
        for (i = 1; i < size-1; i++) {
            for (j = 1; j < size-1; j++) {
                // int t = omp_get_thread_num();
                if (iter == 0 && i == 1 && j == 1) {
                    threads = omp_get_num_threads();
                }
                new_grid[i][j] = 0.25 * (grid[i][j+1] + grid[i][j-1] +
                                         grid[i-1][j] + grid[i+1][j]);

                double diff = new_grid[i][j] - grid[i][j];
                if (diff < 0) {
                    diff = -diff;
                }
                // err = diff;
                if (diff > err) {
                    err = diff;
                }
            }
        }
        
        // swap now and next
        double **swap = grid;
        grid = new_grid;
        new_grid = swap;
        // // copie the next values into the working array for the next iteration
        // for( int i = 1; i < size-1; i++) {
        //     for( int j = 1; j < size-1; j++) {
        //         grid[i][j] = new_grid[i][j];
        //     }
        // }

        if (iter < 3 || iter % (ITER_MAX / 100)== 0) {
            fprintf(stderr, "Error of %le at iteration %d\n", err, iter);
        }

        if (iter % (ITER_MAX / 100) == 0) {
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
    for(int i = 0; i < size; i++){
        for(int j = 0; j < size; j++){
            printf("%lf ", grid[i][j]);
        }
        printf("\n");
    }
    printf("\n");

    // get the end time
    gettimeofday(&time_end, NULL);
    double exec_time = (double) (time_end.tv_sec - time_start.tv_sec) +
                       (double) (time_end.tv_usec - time_start.tv_usec) / 1000000.0;

    fprintf(stderr, "\n%s %dx%d executed %lfs, %d iter, %le err, %d thr.\n", argv[0], size, size, exec_time, iter, err, threads);

    return 0;
}
