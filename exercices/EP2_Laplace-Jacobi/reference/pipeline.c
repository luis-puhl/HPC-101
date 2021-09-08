#include <cstddef>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <err.h>
#include <sys/time.h>
#include <pthread.h>

#define ERR(v) if((v) != 0) errx(EXIT_FAILURE, "Assert error. At "__FILE__":%d\n", __LINE__)

typedef struct thread_args_st{
    size_t currRow, rowAvailable, iter, nThreads, threadId, *solverThread;
    pthread_mutex_t lock;
    pthread_cond_t rowRequest, interationComplete, *problemComplete;
    struct thread_args_st *next;
} Args;

void *work(void *args) {
    Args *a = args;
}

int main(int argc, char *argv[]) {
    size_t nThreads = argc > 2 ? atoi(argv[1]) : 4;
    pthread_t threads[nThreads];
    Args args[nThreads];
    for (int i = 0; i < nThreads; i++) {
        ERR(pthread_create(&threads[i], NULL, &work, &args[i]));
    }
}
