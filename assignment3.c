#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define BUFFER_SIZE 8
#define PRODUCERS 3
#define CONSUMERS 3
#define ITEMS_PER_PRODUCER 50
#define ITEMS_PER_CONSUMER (PRODUCERS * ITEMS_PER_PRODUCER / CONSUMERS)

int buffer[BUFFER_SIZE];
int in_pos = 0;
int out_pos = 0;

sem_t empty_slots;
sem_t full_slots;
pthread_mutex_t buffer_mutex;

int produced = 0;
int consumed = 0;

void* producer(void* arg) {
    int id = (long)arg;
    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        sem_wait(&empty_slots);
        pthread_mutex_lock(&buffer_mutex);

        buffer[in_pos] = id * 1000 + i;
        in_pos = (in_pos + 1) % BUFFER_SIZE;
        produced++;

        pthread_mutex_unlock(&buffer_mutex);
        sem_post(&full_slots);
    }
    return NULL;
}

void* consumer(void* arg) {
    (void)arg;
    for (int i = 0; i < ITEMS_PER_CONSUMER; i++) {
        sem_wait(&full_slots);
        pthread_mutex_lock(&buffer_mutex);

        out_pos = (out_pos + 1) % BUFFER_SIZE;
        consumed++;

        pthread_mutex_unlock(&buffer_mutex);
        sem_post(&empty_slots);
    }
    return NULL;
}

int main() {
    sem_init(&empty_slots, 0, BUFFER_SIZE);
    sem_init(&full_slots, 0, 0);
    pthread_mutex_init(&buffer_mutex, NULL);

    pthread_t prod[PRODUCERS], cons[CONSUMERS];

    for (long i = 0; i < PRODUCERS; i++)
        pthread_create(&prod[i], NULL, producer, (void*)i);

    for (int i = 0; i < CONSUMERS; i++)
        pthread_create(&cons[i], NULL, consumer, NULL);

    for (int i = 0; i < PRODUCERS; i++)
        pthread_join(prod[i], NULL);

    for (int i = 0; i < CONSUMERS; i++)
        pthread_join(cons[i], NULL);

    printf("Produced: %d\n", produced);
    printf("Consumed: %d\n", consumed);

    pthread_mutex_destroy(&buffer_mutex);
    sem_destroy(&empty_slots);
    sem_destroy(&full_slots);
    return 0;
}

