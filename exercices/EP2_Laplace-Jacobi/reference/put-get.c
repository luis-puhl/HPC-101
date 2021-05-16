#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

typedef int Task;

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t isEmpty, isFull;
    int length, size, head, tail, *values;
} Queue;

void put(Queue *q, int value) {
    pthread_mutex_lock(&q->lock);
    while (q->size == q->length) {
        pthread_cond_wait(&q->isEmpty, &q->lock);
    }
    q->values[q->tail] = value;
    q->tail = (q->tail + 1) % q->length;
    q->size++;
    pthread_cond_signal(&q->isFull);
    pthread_mutex_unlock(&q->lock);
}
int get(Queue *q) {
    pthread_mutex_lock(&q->lock);
    while (q->size == 0) {
        pthread_cond_wait(&q->isFull, &q->lock);
    }
    int value = q->values[q->head];
    q->head = (q->head + 1) % q->length;
    q->size--;
    pthread_cond_signal(&q->isEmpty);
    pthread_mutex_unlock(&q->lock);
    return value;
}

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
    Queue q = { .size = 0, .head = 0, .tail = 0, .length = 6 };
    q.values = calloc(6, sizeof(int));
    
    pthread_cond_init(&q.isEmpty, NULL);
    pthread_cond_init(&q.isFull, NULL);
    pthread_mutex_init(&q.lock, NULL);
    
    pthread_t prod, cons;
    pthread_create(&prod, NULL, &producer, &q);
    pthread_create(&cons, NULL, &consumer, &q);

    pthread_join(prod, NULL);
    pthread_join(cons, NULL);
}
