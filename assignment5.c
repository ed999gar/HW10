#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define K 3
#define N 10

sem_t printers;
pthread_mutex_t counter_mutex;

int active = 0;

void* print_job(void* arg) {
    int id = (long)arg;

    sem_wait(&printers);

    pthread_mutex_lock(&counter_mutex);
    active++;
    printf("Thread %d starts printing (active = %d)\n", id, active);
    pthread_mutex_unlock(&counter_mutex);

    usleep(200000);

    pthread_mutex_lock(&counter_mutex);
    active--;
    printf("Thread %d finished printing (active = %d)\n", id, active);
    pthread_mutex_unlock(&counter_mutex);

    sem_post(&printers);

    return NULL;
}

int main() {
    sem_init(&printers, 0, K);
    pthread_mutex_init(&counter_mutex, NULL);

    pthread_t threads[N];

    for (long i = 0; i < N; i++)
        pthread_create(&threads[i], NULL, print_job, (void*)i);

    for (int i = 0; i < N; i++)
        pthread_join(threads[i], NULL);

    pthread_mutex_destroy(&counter_mutex);
    sem_destroy(&printers);

    return 0;
}

