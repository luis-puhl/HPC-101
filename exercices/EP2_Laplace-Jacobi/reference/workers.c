#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

typedef struct {
    Queue input, output;
    int done;
} ThreadArgs;

void * worker(void *arg) {
    ThreadArgs *state = arg;
    fprintf(stderr, "worker ready\n");
    while (!state->done) {
        int col = get(&state->input);
        col++;
        put(&state->output, col);
    }
    fprintf(stderr, "worker done\n");
    return arg;
}

int main(int argc, char *argv[]) {
    int n_jobs = 4;
    if (argc >= 2) {
      n_jobs = atoi(argv[1]);
    }
    int n_workers = 4;
    if (argc >= 3) {
        n_workers = atoi(argv[2]);
    }
    //
    ThreadArgs state = { .done = 0 };
    queue_init(&state.input, n_jobs);
    queue_init(&state.output, n_jobs);
    
    pthread_t workers[n_workers];
    for (size_t i = 0; i < n_workers; i++) {
        pthread_create(&workers[i], NULL, &worker, &state);
    }

    for (int i = 0; i < n_workers; i++) {
        fprintf(stderr, "i=%d\n", i);
        for (int j = 0; j < n_jobs; j++) {
            put(&state.input, j);
        }
        int max = 0;
        for (int j = 0; j < n_jobs; j++) {
            int v = get(&state.output);
            if (v > max) {
                max = v;
            }
        }
        fprintf(stderr, "i=%d, max=%d\n", i, max);
    }
    state.done = 1;
    for (int j = 0; j < n_workers; j++) {
        put(&state.input, 0);
    }

    for (size_t i = 0; i < n_workers; i++) {
        pthread_join(workers[i], NULL);
    }
}
