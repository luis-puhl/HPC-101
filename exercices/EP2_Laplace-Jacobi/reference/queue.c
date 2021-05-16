#include <pthread.h>
#include <stdlib.h>

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

Queue* queue_init(Queue *queue, int length) {
    Queue q = { .size = 0, .head = 0, .tail = 0 };
    q.length = length;
    q.values = calloc(length, sizeof(int));

    pthread_cond_init(&q.isEmpty, NULL);
    pthread_cond_init(&q.isFull, NULL);
    pthread_mutex_init(&q.lock, NULL);
    
    *queue = q;
    return queue;
}
