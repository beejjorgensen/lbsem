#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>

sem_t *mutex, *barrier;

const int n = 10;  // Number of threads
int count = 0;

sem_t *sem_open_temp(const char *name, int value)
{
    sem_t *sem;

    // Create the semaphore
    if ((sem = sem_open(name, O_CREAT, 0666, value)) == SEM_FAILED)
        return SEM_FAILED;

    // Unlink it so it will go away after this process exits
    if (sem_unlink(name) == -1) {
        sem_close(sem);
        return SEM_FAILED;
    }

    return sem;
}

void *threads(void *arg)
{
    int *tid = arg;

    sem_wait(mutex);
    count = count + 1;
    sem_post(mutex);

    if (count == n) sem_post(barrier);

    // Turnstile
    sem_wait(barrier);
    sem_post(barrier);

    printf("thread %d: critical point\n", *tid);

    return NULL;
}

int main(void)
{
    pthread_t t[n];
    int tid[n];

    // Create the semaphore
    mutex = sem_open_temp("sem_prog3.6_m", 1);
    barrier = sem_open_temp("sem_prog3.6_b", 0);

    // Create threads
    for (int i = 0; i < n; i++) {
        tid[i] = i;
        pthread_create(t + i, NULL, threads, tid + i);
    }

    // Wait for completion
    for (int i = 0; i < n; i++)
        pthread_join(t[i], NULL);

    // Close the semaphore
    sem_close(mutex);
    sem_close(barrier);
}
