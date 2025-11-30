#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define DEFAULT_THREADS 4
#define DEFAULT_ITERATIONS 1000000LL

long long counter = 0;

pthread_mutex_t counter_mutex;
pthread_spinlock_t counter_spin;

typedef enum {
    MODE_RACE_ONLY,
    MODE_USE_MUTEX,
    MODE_USE_SPIN
} sync_mode;

sync_mode g_mode;
long long g_iterations = DEFAULT_ITERATIONS;
int g_threads = DEFAULT_THREADS;

void* worker(void* arg) {
    (void)arg;
    for (long long i = 0; i < g_iterations; i++) {
        if (g_mode == MODE_RACE_ONLY) {
            counter++;
        } else if (g_mode == MODE_USE_MUTEX) {
            pthread_mutex_lock(&counter_mutex);
            counter++;
            pthread_mutex_unlock(&counter_mutex);
        } else {
            pthread_spin_lock(&counter_spin);
            counter++;
            pthread_spin_unlock(&counter_spin);
        }
    }
    return NULL;
}

sync_mode parse_mode(const char* s) {
    if (strcmp(s, "race") == 0) return MODE_RACE_ONLY;
    if (strcmp(s, "mutex") == 0) return MODE_USE_MUTEX;
    if (strcmp(s, "spin") == 0) return MODE_USE_SPIN;
    fprintf(stderr, "unknown mode\n");
    exit(1);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <mode> [threads] [iterations]\n", argv[0]);
        return 1;
    }

    g_mode = parse_mode(argv[1]);

    if (argc >= 3) g_threads = atoi(argv[2]);
    if (argc >= 4) g_iterations = atoll(argv[3]);

    if (g_mode == MODE_USE_MUTEX) pthread_mutex_init(&counter_mutex, NULL);
    if (g_mode == MODE_USE_SPIN) pthread_spin_init(&counter_spin, PTHREAD_PROCESS_PRIVATE);

    pthread_t* threads = malloc(sizeof(pthread_t) * g_threads);
    counter = 0;

    for (int i = 0; i < g_threads; i++)
        pthread_create(&threads[i], NULL, worker, NULL);

    for (int i = 0; i < g_threads; i++)
        pthread_join(threads[i], NULL);

    if (g_mode == MODE_USE_MUTEX) pthread_mutex_destroy(&counter_mutex);
    if (g_mode == MODE_USE_SPIN) pthread_spin_destroy(&counter_spin);

    long long expected = g_threads * g_iterations;

    printf("Expected: %lld\n", expected);
    printf("Actual:   %lld\n", counter);

    free(threads);
    return 0;
}

