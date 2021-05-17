/*
    This program solves Laplace's equation on a regular 2D grid using simple Jacobi iteration.
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <err.h>
#include <sys/time.h>
#include <pthread.h>

#define ERR(v) if((v) != 0) errx(EXIT_FAILURE, "Assert error. At "__FILE__":%d\n", __LINE__)

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t isEmpty, isFull;
    size_t length, size, head, tail;
} Queue;

#define ITER_MAX 50000         // number of maximum iterations
#define CONV_THRESHOLD 1.0e-8f // threshold of convergence

typedef struct {
    double **now, **next;
    Queue input, output;
    int *todo;
    double *done;
    int size;
} Grids;

typedef struct {
    int done;
    Grids *state;
} ThreadArgs;

/**
 * consumer / Worker
 */
static void *processor(void *void_args) {
    ThreadArgs* args = void_args;
    int iterations = 0;
    while (!args->done) {
        // 
        // int i = get(&args->state->input);
        Queue *q = &args->state->input;
        ERR(pthread_mutex_lock(&q->lock));
        while (q->size == 0) {
            ERR(pthread_cond_wait(&q->isFull, &q->lock));
        }
        int i = args->state->todo[q->head];
        q->head = (q->head + 1) % q->length;
        q->size--;
        ERR(pthread_cond_signal(&q->isEmpty));
        ERR(pthread_mutex_unlock(&q->lock));
        // fprintf(stderr, "processor get(%d)\n", i);
        if (args->done == 1) break;
        //
        double err = 0.0;
        for(int j = 1; j < args->state->size - 1; j++) {
            double **now = args->state->now, **next = args->state->next;
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
        // fprintf(stderr, "err=%lf\n", err);
        // put(&args->state->output, err);
        q = &args->state->output;
        // fprintf(stderr, "put(%lf)\n", err);
        ERR(pthread_mutex_lock(&q->lock));
        while (q->size == q->length) {
            ERR(pthread_cond_wait(&q->isEmpty, &q->lock));
        }
        // q->values[q->tail] = err;
        args->state->done[q->tail] = err;
        q->tail = (q->tail + 1) % q->length;
        q->size++;
        ERR(pthread_cond_signal(&q->isFull));
        ERR(pthread_mutex_unlock(&q->lock));
        iterations++;
    }
    // fprintf(stderr, "worker done with i=%d\n", iterations);
    return void_args;
}

int main(int argc, char *argv[]){
    // seed for random generator
    srand(10);
    if(argc != 3){
        printf("Usage: ./laplace_seq N T\n");
        printf("N: The size of each side of the domain (grid)\n");
        printf("T: Number of threads\n");
        exit(-1);
    }
    
    Grids grids;
    grids.size = atoi(argv[1]);
    grids.now = (double **)calloc(grids.size, sizeof(double *));
    grids.next = (double **)calloc(grids.size, sizeof(double *));
    grids.todo = (int*)calloc(grids.size, sizeof(int));
    grids.done = (double*)calloc(grids.size, sizeof(double));

    grids.input.size = 0;
    grids.input.head = 0;
    grids.input.tail = 0;
    grids.input.length = grids.size;
    ERR(pthread_cond_init(&grids.input.isEmpty, NULL));
    ERR(pthread_cond_init(&grids.input.isFull, NULL));
    ERR(pthread_mutex_init(&grids.input.lock, NULL));
    
    grids.output.size = 0;
    grids.output.head = 0;
    grids.output.tail = 0;
    grids.output.length = grids.size;
    ERR(pthread_cond_init(&grids.output.isEmpty, NULL));
    ERR(pthread_cond_init(&grids.output.isFull, NULL));
    ERR(pthread_mutex_init(&grids.output.lock, NULL));

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
    int num_threads = grids.size - 2;
    pthread_t threads[num_threads];
    ThreadArgs params[num_threads];
    for (int thread = 0; thread < num_threads; thread++) {
        params[thread].done = 0;
        params[thread].state = &grids;
        ERR(pthread_create(&threads[thread], NULL, &processor, (void *)&params[thread]));
    }

    struct timeval time_start, time_end;
    gettimeofday(&time_start, NULL);

    // Jacobi iteration
    // This loop will end if either the maximum change reaches below a set threshold (convergence)
    // or a fixed number of maximum iterations have completed
    double err = 0.0;
    int iter;
    for (iter = 0; iter <= ITER_MAX; iter++) {
        for (int i = 1; i < grids.size - 1; i++) {
            // put(&grids.input, i);
            Queue *q = &grids.input;
            // fprintf(stderr, "main put(%d)\n", i);
            ERR(pthread_mutex_lock(&q->lock));
            while (q->size == q->length) {
                ERR(pthread_cond_wait(&q->isEmpty, &q->lock));
            }
            grids.todo[q->tail] = i;
            q->tail = (q->tail + 1) % q->length;
            q->size++;
            ERR(pthread_cond_signal(&q->isFull));
            ERR(pthread_mutex_unlock(&q->lock));
        }
        // gather error
        err = 0.0;
        for (int i = 1; i < grids.size - 1; i++) {
            // double colErr = get(&grids.output);
            Queue *q = &grids.output;
            ERR(pthread_mutex_lock(&q->lock));
            while (q->size == 0) {
              ERR(pthread_cond_wait(&q->isFull, &q->lock));
            }
            double colErr = grids.done[q->head];
            q->head = (q->head + 1) % q->length;
            q->size--;
            ERR(pthread_cond_signal(&q->isEmpty));
            ERR(pthread_mutex_unlock(&q->lock));
            // fprintf(stderr, "main get(%lf)\n", colErr);
            // if (colErr > err) {
            //     err = colErr;
            // }
            err += colErr;
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
    
    // deactivate the threads
    // fprintf(stderr, "deactivate the threads\n");
    // put(&grids.input, 0);
    Queue *q = &grids.input;
    // fprintf(stderr, "put(%d)\n", i);
    ERR(pthread_mutex_lock(&q->lock));
    q->tail = (q->tail + num_threads) % q->length;
    q->size += num_threads;
    for (int thread = 0; thread < num_threads; thread++) {
        grids.todo[q->tail] = 0;
        params[thread].done = 1;
        ERR(pthread_cond_signal(&q->isFull));
    }
    ERR(pthread_mutex_unlock(&q->lock));
    // join
    for (int thread = 0; thread < num_threads; thread++) {
        void *retval;
        ERR(pthread_join(threads[thread], &retval));
    }

    return 0;
}
