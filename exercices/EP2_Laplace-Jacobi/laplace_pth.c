/*
    This program solves Laplace's equation on a regular 2D grid using simple Jacobi iteration.
*/
#include "./laplace.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <err.h>
#include <sys/time.h>
#include <pthread.h>

#define ERR(v) if((v) != 0) errx(EXIT_FAILURE, "Assert error. At "__FILE__":%d\n", __LINE__)
typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t start, done;
    size_t nThreads, nDone;
} Signals;

typedef struct {
    int threadId, indexStart, indexEnd;
    double error;
    Grid *state;
    Signals *signal;
} ThreadArgs;

/**
 * consumer / Worker
 */
static void *processor(void *void_args) {
    ThreadArgs* args = void_args;
    int iterations = 0;
    Signals *signal = args->signal;
    ERR(pthread_cond_signal(&signal->done));
    // fprintf(stderr, "[%d] ready\n", args->threadId);
    while (1) {
        ERR(pthread_mutex_lock(&signal->lock));
        // while (args->signal->nStarted != 0) {
            ERR(pthread_cond_wait(&signal->start, &signal->lock));
        // }
        // fprintf(stderr, "[%d] start iter=%d\n", args->threadId, iterations);
        ERR(pthread_mutex_unlock(&signal->lock));
        if (args->signal->nThreads == 0) {
            break;
        }
        double err = 0.0;
        for (int index = args->indexStart; index < args->indexEnd; index++) {
            size_t i = index / args->state->size;
            size_t j = index % args->state->size;
            double diff = laplace_iterate_cell(args->state, i, j);
            if (diff > err) {
                err = diff;
            }
        }
        // fprintf(stderr, "[%d] done iter=%d\n", args->threadId, iterations);
        ERR(pthread_mutex_lock(&signal->lock));
        args->error = err;
        signal->nDone++;
        ERR(pthread_cond_signal(&signal->done));
        ERR(pthread_mutex_unlock(&signal->lock));
        iterations++;
    }
    // fprintf(stderr, "worker done with i=%d\n", iterations);
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
    
    Signals signal;
    signal.nThreads = atoi(argv[2]);
    signal.nDone = 0;
    ERR(pthread_cond_init(&signal.start, NULL));
    ERR(pthread_cond_init(&signal.done, NULL));
    ERR(pthread_mutex_init(&signal.lock, NULL));
    pthread_t threads[signal.nThreads];
    ThreadArgs params[signal.nThreads];
    size_t total = grid.size * grid.size;
    size_t part = total / signal.nThreads;
    for (int thread = 0; thread < signal.nThreads; thread++) {
        params[thread].threadId = thread;
        params[thread].indexStart = thread * part;
        params[thread].indexEnd = (thread + 1) * part;
        if (thread == signal.nThreads -1) {
            params[thread].indexEnd = total;
        }
        params[thread].error = 0.0;
        params[thread].signal = &signal;
        params[thread].state = &grid;
        ERR(pthread_create(&threads[thread], NULL, &processor, (void *)&params[thread]));
        // wait for them to be ready
        ERR(pthread_mutex_lock(&signal.lock));
        ERR(pthread_cond_wait(&signal.done, &signal.lock));
        ERR(pthread_mutex_unlock(&signal.lock));
    }

    struct timeval time_start, time_end;
    gettimeofday(&time_start, NULL);

    // Jacobi iteration
    // This loop will end if either the maximum change reaches below a set threshold (convergence)
    // or a fixed number of maximum iterations have completed
    double err = 0.0;
    int iter;
    for (iter = 0; iter <= ITER_MAX; iter++) {
        ERR(pthread_mutex_lock(&signal.lock));
        signal.nDone = 0;
        fprintf(stderr, "broadcast\n");
        ERR(pthread_mutex_unlock(&signal.lock));
        ERR(pthread_cond_broadcast(&signal.start));
        //
        sched_yield();
        //
        ERR(pthread_mutex_lock(&signal.lock));
        while (signal.nDone != signal.nThreads) {
            ERR(pthread_cond_wait(&signal.done, &signal.lock));
        }
        fprintf(stderr, "gather error\n");
        // gather error
        err = 0.0;
        for (int thread = 0; thread < signal.nThreads; thread++) {
            if (err < params[thread].error) {
                err = params[thread].error;
            }
            params[thread].error = 0.0;
        }
        //
        double **swap_grid = grid.now;
        grid.now = grid.next;
        grid.next = swap_grid;
        ERR(pthread_mutex_unlock(&signal.lock));
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
    fprintf(stderr, "\n%s Kernel executed in %le seconds with %d iterations and error of %le\n", argv[0], exec_time, iter, err);
    
    // deactivate the threads
    ERR(pthread_mutex_lock(&signal.lock));
    signal.nThreads = 0;
    ERR(pthread_cond_broadcast(&signal.start));
    ERR(pthread_mutex_unlock(&signal.lock));
    // join
    for (int thread = 0; thread < signal.nThreads; thread++) {
        ERR(pthread_join(threads[thread], NULL));
    }

    return 0;
}
