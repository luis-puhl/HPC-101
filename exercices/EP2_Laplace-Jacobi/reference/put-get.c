#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#include "queue.h"

void * producer(void *arg) {
    Queue *q = arg;
    for (int i = 0; i <= 10; i++) {
        put(q, i);
    }
    fprintf(stderr, "producer done\n");
    return arg;
}

void * consumer(void *arg) {
    Queue *q = arg;
    int i;
    do {
        i = get(q);
        printf("t=%d\n", i);
    } while (i < 10);
    fprintf(stderr, "consumer done\n");
    return arg;
}

int main(int argc, char *argv[]) {
    Queue q;
    queue_init(&q, 6);
    
    pthread_t prod, cons;
    pthread_create(&prod, NULL, &producer, &q);
    pthread_create(&cons, NULL, &consumer, &q);

    pthread_join(prod, NULL);
    pthread_join(cons, NULL);
}
