#include <stdio.h>
#include <pthread.h>

typedef int Task;

struct State {
    pthread_mutex_t task_queue_cond_lock;
    pthread_cond_t cond_queue_empty, cond_queue_full;
    int task_available;
    Task task;
};

#define done() (inserted > 10)
#define create_task() inserted++
#define insert_into_queue(t) (s->task = t)
#define extract_from_queue() inserted = s->task
#define process_task(task) printf("t=%d\n", task)

void * producer(void *arg) {
    struct State *s = arg;
    int inserted;
    while (!done()) {
        Task my_task = create_task();
        pthread_mutex_lock(&s->task_queue_cond_lock);
        while (s->task_available == 1) {
            pthread_cond_wait(&s->cond_queue_empty, &s->task_queue_cond_lock);
        }
        insert_into_queue(my_task);
        s->task_available = 1;
        pthread_cond_signal(&s->cond_queue_full);
        pthread_mutex_unlock(&s->task_queue_cond_lock);
    }
    fprintf(stderr, "producer done\n");
    return arg;
}

void * consumer(void *arg) {
    struct State *s = arg;
    int inserted;
     while (!done()) {
        pthread_mutex_lock(&s->task_queue_cond_lock);
        while (s->task_available == 0) {
            pthread_cond_wait(&s->cond_queue_full, &s->task_queue_cond_lock);
        }
        Task my_task = extract_from_queue();
        s->task_available = 0;
        pthread_cond_signal(&s->cond_queue_empty);
        pthread_mutex_unlock(&s->task_queue_cond_lock);
        inserted = my_task;
        if (my_task >= 10) break;
        process_task(my_task);
    }
    fprintf(stderr, "consumer done\n");
    return arg;
}

int main(int argc, char *argv[]) {
    struct State state;
    state.task_available = 0;
    pthread_cond_init(&state.cond_queue_empty, NULL);
    pthread_cond_init(&state.cond_queue_full, NULL);
    pthread_mutex_init(&state.task_queue_cond_lock, NULL);
    
    pthread_t prod, cons;
    pthread_create(&prod, NULL, &producer, &state);
    pthread_create(&cons, NULL, &consumer, &state);

    pthread_join(prod, NULL);
    pthread_join(cons, NULL);
}
