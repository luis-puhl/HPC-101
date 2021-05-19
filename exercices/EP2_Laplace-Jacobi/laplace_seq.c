/*
    This program solves Laplace's equation on a regular 2D grid using simple Jacobi iteration.
*/
#include "./laplace.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

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
    Grid grid = laplace_init(atoi(argv[1]));
    fprintf(stderr, "Jacobi relaxation calculation: %lu x %lu grid\t%s\n", grid.size, grid.size, argv[0]);

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
        for( int i = 1; i < grid.size-1; i++) {
            for (int j = 1; j < grid.size-1; j++) {
                double diff = laplace_iterate_cell(&grid, i, j);
                if (diff > err) {
                    err = diff;
                }
            }
        }
        double **swap_grid = grid.now;
        grid.now = grid.next;
        grid.next = swap_grid;
        laplace_print(&grid);
        if (err < CONV_THRESHOLD) {
            fprintf(stderr, "main break iteration=%d, err=%le\n", iter, err);
            break;
        }

        if (iter < 3 || (iter % (ITER_MAX / 100)) == 0) {
            fprintf(stderr, "iteration=%d, err=%le\n", iter, err);
            // fprintf(stderr, "Root, grid(%p, %p)\n", grids.now, grids.next);
        }
    }

    // get the end time
    gettimeofday(&time_end, NULL);

    double exec_time = (double) (time_end.tv_sec - time_start.tv_sec) +
                       (double) (time_end.tv_usec - time_start.tv_usec) / 1000000.0;

    fprintf(stderr, "\n%s Kernel executed in %le seconds with %d iterations and error of %le\n", argv[0], exec_time, iter, err);

    return 0;
}
