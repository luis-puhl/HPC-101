#include <pthread.h>

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t isEmpty, isFull;
    int length, size, head, tail, *values;
} Queue;

void put(Queue *q, int value);
int get(Queue *q);
Queue *queue_init(Queue *queue, int length);
