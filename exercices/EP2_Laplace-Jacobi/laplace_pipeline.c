/*
    This program solves Laplace's equation on a regular 2D grid using simple Jacobi iteration.
*/
#include "./laplace.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <err.h>
#include <sys/time.h>
#include <pthread.h>

#define ERR(v) if((v) != 0) errx(EXIT_FAILURE, "Assert error. At "__FILE__":%d\n", __LINE__)

typedef struct thread_args_st{
    double error;
    Grid grid;
    size_t currRow, rowAvailable, iter, nThreads, threadId, *solverThread;
    pthread_mutex_t lock;
    pthread_cond_t rowRequest, interationComplete, *problemComplete;
    struct thread_args_st *next;
} ThreadArgs;

int getNextRow(size_t row, Grid *g, ThreadArgs *t) {
    ERR(pthread_mutex_lock(&t->lock));
    if (t->solverThread != 0) return 0;
    while (t->currRow <= row) {
        ERR(pthread_cond_wait(&t->rowRequest, &t->lock));
    }
    memcpy(&g->now[row *g->size], &t->grid.next[row * g->size], g->size * sizeof(double));
    ERR(pthread_mutex_unlock(&t->lock));
    return 1;
}

static void *processor(void *void_args) {
    ThreadArgs* args = void_args;
    size_t size = args->grid.size;
    for (; args->iter <= ITER_MAX; args->iter += args->nThreads) {
        args->error = 0.0;
        if (args->rowAvailable == 0) {
            if (!getNextRow(1, &args->grid, args->next)) {
                return void_args;
            }
        }
        for (int i = 1; i < size -1; i++) {
            if (i + 1 >= args->rowAvailable) {
                if (!getNextRow(i + 1, &args->grid, args->next)) {
                    return void_args;
                }
            }
            for (int j = 1; j < size -1; j++) {
                double diff = laplace_iterate_cell(&args->grid, i, j);
                if (diff > args->error) {
                    args->error = diff;
                }
            }
            ERR(pthread_mutex_lock(&args->lock));
            args->currRow = i;
            ERR(pthread_mutex_unlock(&args->lock));
            ERR(pthread_cond_signal(&args->rowRequest));
        }
        ERR(pthread_mutex_lock(&args->lock));
        args->currRow = i;
        ERR(pthread_mutex_unlock(&args->lock));
        ERR(pthread_cond_signal(&args->interationComplete));
        args->rowAvailable = 0;
        if (args->error < CONV_THRESHOLD) {
            break;
        }
    }
    if (args->iter <= ITER_MAX) {
        ERR(pthread_cond_signal(args->problemComplete));
        *args->solverThread = args->threadId;
    }
    return void_args;
}

int main(int argc, char *argv[]) {
    if(argc != 3){
        printf("Usage: ./laplace_seq N T\n");
        printf("N: The size of each side of the domain (grid)\n");
        printf("T: Number of threads\n");
        exit(-1);
    }
    
    // size of each side of the grid
    Grid grid = laplace_init(atoi(argv[1]));
    fprintf(stderr, "Jacobi relaxation calculation: %lu x %lu grid\t%s\n", grid.size, grid.size, argv[0]);
    
    size_t nThreads = atoi(argv[2]);
    pthread_t threads[nThreads];
    ThreadArgs params[nThreads];
    //
    size_t solverThread = nThreads;
    pthread_cond_t problemComplete;
    pthread_mutex_t problemLock;
    ERR(pthread_mutex_init(&problemLock, NULL));
    ERR(pthread_cond_init(&problemComplete, NULL));
    for (int thread = 0; thread < nThreads; thread++) {
        params[thread].threadId = thread;
        params[thread].nThreads = nThreads;
        params[thread].next = &params[(thread + 1) % nThreads];
        params[thread].grid = laplace_init(grid.size);
        params[thread].problemComplete = &problemComplete;
        params[thread].solverThread = &solverThread;
        ERR(pthread_mutex_init(&params[thread].lock, NULL));
        ERR(pthread_cond_init(&params[thread].rowRequest, NULL));
    }
    params[0].rowAvailable = grid.size;
    // not needed, all have the same init procedure
    // memcpy(params[0].grid.now, grid.now, grid.size * grid.size * sizeof(double));


    struct timeval time_start, time_end;
    gettimeofday(&time_start, NULL);

    for (int thread = 0; thread < nThreads; thread++) {
        ERR(pthread_create(&threads[thread], NULL, &processor, (void *)&params[thread]));
    }
    ERR(pthread_mutex_lock(&problemLock));
    ERR(pthread_cond_wait(&problemComplete, &problemLock));
    ERR(pthread_mutex_unlock(&problemLock));

    // Jacobi iteration
    // This loop will end if either the maximum change reaches below a set threshold (convergence)
    // or a fixed number of maximum iterations have completed
    double err = 0.0;
    int iter;
    params[0].grid.now = grid.now;
    for (iter = 0; iter <= ITER_MAX; iter++) {
        int thread = iter % nThreads;
        if (iter > nThreads) {
            ERR(pthread_join(threads[thread], NULL));
            err = params[thread].error;
        }
        ERR(pthread_create(&threads[thread], NULL, &processor, (void *)&params[thread]));
        if (iter < nThreads) {
            continue;
        }
        //
        double **swap_grid = grid.now;
        grid.now = grid.next;
        grid.next = swap_grid;
        // ERR(pthread_mutex_unlock(&signal.lock));
        laplace_print(&grid);
        if (err < CONV_THRESHOLD) {
            fprintf(stderr, "main break iteration=%d, err=%le\n", iter, err);
            break;
        }
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
    laplace_print(&grid);
    fprintf(stderr,
            "\n%s Kernel %lux%lu executed in %le seconds with %d iterations and "
            "error of %le\n",
            argv[0], grid.size, grid.size, exec_time, iter, err);

    // deactivate the threads
    // ERR(pthread_mutex_lock(&signal.lock));
    // nThreads = 0;
    // ERR(pthread_cond_broadcast(&signal.start));
    // ERR(pthread_mutex_unlock(&signal.lock));
    // join
    for (int thread = 0; thread < nThreads; thread++) {
        params[thread].state = NULL;
        params[thread].isReady = 0;
        ERR(pthread_cond_signal(&params[thread].start));
        ERR(pthread_join(threads[thread], NULL));
    }

    return 0;
}
