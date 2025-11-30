#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define NUM_DEPOSIT_THREADS 4
#define NUM_WITHDRAW_THREADS 4
#define DEFAULT_ITERATIONS 100000

long long balance = 0;

pthread_mutex_t balance_mutex;
pthread_spinlock_t balance_spin;

int use_mutex = 1;
int long_cs = 0;
long long iterations = DEFAULT_ITERATIONS;

void lock_balance() {
    if (use_mutex) pthread_mutex_lock(&balance_mutex);
    else pthread_spin_lock(&balance_spin);
}

void unlock_balance() {
    if (use_mutex) pthread_mutex_unlock(&balance_mutex);
    else pthread_spin_unlock(&balance_spin);
}

void* deposit_worker(void* arg) {
    (void)arg;
    for (long long i = 0; i < iterations; i++) {
        lock_balance();
        if (long_cs) usleep(10);
        balance++;
        unlock_balance();
    }
    return NULL;
}

void* withdraw_worker(void* arg) {
    (void)arg;
    for (long long i = 0; i < iterations; i++) {
        lock_balance();
        if (long_cs) usleep(10);
        balance--;
        unlock_balance();
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s <mutex|spin> <short|long> [iterations]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "mutex") == 0) use_mutex = 1;
    else if (strcmp(argv[1], "spin") == 0) use_mutex = 0;
    else {
        fprintf(stderr, "first arg must be mutex or spin\n");
        return 1;
    }

    if (strcmp(argv[2], "short") == 0) long_cs = 0;
    else if (strcmp(argv[2], "long") == 0) long_cs = 1;
    else {
        fprintf(stderr, "second arg must be short or long\n");
        return 1;
    }

    if (argc >= 4) {
        long long tmp = atoll(argv[3]);
        if (tmp > 0) iterations = tmp;
    }

    if (use_mutex) {
        if (pthread_mutex_init(&balance_mutex, NULL) != 0) {
            perror("pthread_mutex_init");
            return 1;
        }
    } else {
        if (pthread_spin_init(&balance_spin, PTHREAD_PROCESS_PRIVATE) != 0) {
            perror("pthread_spin_init");
            return 1;
        }
    }

    pthread_t threads[NUM_DEPOSIT_THREADS + NUM_WITHDRAW_THREADS];
    int idx = 0;
    balance = 0;

    for (int i = 0; i < NUM_DEPOSIT_THREADS; i++) {
        if (pthread_create(&threads[idx++], NULL, deposit_worker, NULL) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    for (int i = 0; i < NUM_WITHDRAW_THREADS; i++) {
        if (pthread_create(&threads[idx++], NULL, withdraw_worker, NULL) != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    int total = NUM_DEPOSIT_THREADS + NUM_WITHDRAW_THREADS;
    for (int i = 0; i < total; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join");
            return 1;
        }
    }

    if (use_mutex) pthread_mutex_destroy(&balance_mutex);
    else pthread_spin_destroy(&balance_spin);

    printf("Mode: %s, CS: %s\n", use_mutex ? "mutex" : "spin", long_cs ? "long" : "short");
    printf("Threads: deposits=%d, withdraws=%d, iterations=%lld\n",
           NUM_DEPOSIT_THREADS, NUM_WITHDRAW_THREADS, iterations);
    printf("Final balance: %lld\n", balance);

    return 0;
}

