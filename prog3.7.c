#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>

sem_t *mutex, *turnstile, *turnstile2;

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

    for (int i = 0; i < 5; i++) {
        sem_wait(mutex);
        count = count + 1;
        if (count == n) {
            sem_wait(turnstile2);   // lock the second
            sem_post(turnstile);    // unlock the first
        }
        sem_post(mutex);

        sem_wait(turnstile);        // first turnstile
        sem_post(turnstile);

        printf("thread %d.%d: critical point\n", *tid, i);

        sem_wait(mutex);
        count = count - 1;
        if (count == 0) {
            sem_wait(turnstile);    // lock the first
            sem_post(turnstile2);   // unlock the second
        }
        sem_post(mutex);

        sem_wait(turnstile2);        // second turnstile
        sem_post(turnstile2);
    }

    return NULL;
}

int main(void)
{
    pthread_t t[n];
    int tid[n];

    // Create the semaphore
    mutex = sem_open_temp("sem_prog3.7_m", 1);
    turnstile = sem_open_temp("sem_prog3.7_t", 0);
    turnstile2 = sem_open_temp("sem_prog3.7_t2", 1);

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
    sem_close(turnstile);
    sem_close(turnstile2);
}
