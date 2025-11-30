#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define N 10

sem_t semA;
sem_t semB;
sem_t semC;

void* threadA(void* arg) {
    for (int i = 0; i < N; i++) {
        sem_wait(&semA);
        printf("A %d\n", i);
        sem_post(&semB);
    }
    return NULL;
}

void* threadB(void* arg) {
    for (int i = 0; i < N; i++) {
        sem_wait(&semB);
        printf("B %d\n", i);
        sem_post(&semC);
    }
    return NULL;
}

void* threadC(void* arg) {
    for (int i = 0; i < N; i++) {
        sem_wait(&semC);
        printf("C %d\n", i);
        sem_post(&semA);
    }
    return NULL;
}

int main() {
    sem_init(&semA, 0, 1);
    sem_init(&semB, 0, 0);
    sem_init(&semC, 0, 0);

    pthread_t A, B, C;
    pthread_create(&A, NULL, threadA, NULL);
    pthread_create(&B, NULL, threadB, NULL);
    pthread_create(&C, NULL, threadC, NULL);

    pthread_join(A, NULL);
    pthread_join(B, NULL);
    pthread_join(C, NULL);

    sem_destroy(&semA);
    sem_destroy(&semB);
    sem_destroy(&semC);
    return 0;
}

