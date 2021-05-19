/*
    This program solves Laplace's equation on a regular 2D grid using simple Jacobi iteration.
*/
#include "laplace.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <err.h>
#include <sys/time.h>

#define ERR(v) if((v) != 0) errx(EXIT_FAILURE, "Assert error. At "__FILE__":%d\n", __LINE__)

typedef struct {
    double **now, **next;
    int size;
} Grids;

int main(int argc, char *argv[]){
    // seed for random generator
    srand(10);
    if(argc != 2){
        printf("Usage: ./laplace_seq N T\n");
        printf("N: The size of each side of the domain (grid)\n");
        exit(-1);
    }
    
    Grids grids;
    grids.size = atoi(argv[1]);
    grids.now = (double **)calloc(grids.size, sizeof(double *));
    grids.next = (double **)calloc(grids.size, sizeof(double *));

    for (int i = 0; i < grids.size; i++){
        grids.now[i] = (double *) calloc(grids.size, sizeof(double));
        grids.next[i] = (double *) calloc(grids.size, sizeof(double));
        // set grid initial conditions
        for (int j = 0; j < grids.size; j++){
            grids.now[i][j] = rand() % 100;
            grids.next[i][j] = grids.now[i][j];
            printf("%lf ", grids.now[i][j]);
        }
        printf("\n");
    }
    printf("Jacobi relaxation calculation: %d x %d grid\t%s\n", grids.size, grids.size, argv[0]);
    fprintf(stderr, "Root, grid(%p, %p)\n", grids.now, grids.next);

    // int num_threads = atoi(argv[2]);
    // num_threads = (size - 2) < num_threads ? (size - 2) : num_threads;

    struct timeval time_start, time_end;
    gettimeofday(&time_start, NULL);

    // Jacobi iteration
    // This loop will end if either the maximum change reaches below a set threshold (convergence)
    // or a fixed number of maximum iterations have completed
    double err = 0.0;
    int iter;
    for (iter = 0; iter <= ITER_MAX; iter++) {
        err = 0.0;
        for (int i = 1; i < grids.size - 1; i++) {
            for(int j = 1; j < grids.size - 1; j++) {
                double **now = grids.now, **next = grids.next;
                next[i][j] = 0.25 * (now[i][j + 1] + now[i][j - 1]
                                    + now[i - 1][j] + now[i + 1][j]);
                // 
                double diff = next[i][j] - now[i][j];
                if (diff < 0) {
                    diff *= -1.0;
                }
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
        // 
        double **swap_grid = grids.now;
        grids.now = grids.next;
        grids.next = swap_grid;
        //
        if (iter < 3 || (iter % (ITER_MAX / 100)) == 0) {
          fprintf(stderr, "iteration=%d, err=%0.10lf\n", iter, err);
          // fprintf(stderr, "Root, grid(%p, %p)\n", grids.now, grids.next);
        }
    }

    gettimeofday(&time_end, NULL);
    double exec_time = (double) (time_end.tv_sec - time_start.tv_sec) +
                       (double) (time_end.tv_usec - time_start.tv_usec) / 1000000.0;
    //
    double **swap_grid = grids.now;
    grids.now = grids.next;
    grids.next = swap_grid;
    for(int i = 0; i < grids.size; i++){
        for(int j = 0; j < grids.size; j++){
            printf("%lf ", grids.now[i][j]);
        }
        printf("\n");
    }
    printf("\n%s Kernel executed in %le seconds with %d iterations and error of %le\n", argv[0], exec_time, iter, err);
    return 0;
}
