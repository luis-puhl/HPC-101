#include <stdio.h>
#include <pthread.h>

void * producer(void *arg) {
    int inserted;
    while (!done()) {
        create_task();
        pthread_mutex_lock(&task_queue_cond_lock);
        while (task_available == 1) {
            pthread_cond_wait(*cond_queue_empty, &task_queue_cond_lock);
        }
        insert_into_queue();
        task_available = 1;
        pthread_cond_signal(&cond_queue_full);
        pthread_mutex_unlock(&task_queue_cond_lock);
    }
    return arg;
}

void * consumer(void *arg) {
     while (!done()) {
        pthread_mutex_lock(&task_queue_cond_lock);
        while (task_available == 1) {
            pthread_cond_wait(*cond_queue_full, &task_queue_cond_lock);
        }
        my_task = extract_from_queue();
        task_available = 0;
        pthread_cond_signal(&cond_queue_empty);
        pthread_mutex_unlock(&task_queue_cond_lock);
        process_task(my_task);
    }
    return arg;
}

int main(int argc, char *argv[]) {
    // 
}
