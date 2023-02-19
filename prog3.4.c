#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>

sem_t *mutex;
int count = 0;

void *threads(void *arg)
{
    (void)arg;

    if (sem_wait(mutex) == -1) {
        perror("sem_wait");
    }

    count = count + 1;

    if (sem_post(mutex) == -1) {
        perror("sem_post");
    }

    return NULL;
}

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

int main(void)
{
    pthread_t ta, tb;

    // Create the semaphore
    if ((mutex = sem_open_temp("sem_prog3.4", 1)) == SEM_FAILED) {
        perror("sem_open_temp");
        return 1;
    }

    printf("Count before: %d\n", count);

    // Make both threads--make them in the wrong order to encourage "b"
    // to execute first.
    pthread_create(&tb, NULL, threads, NULL);
    pthread_create(&ta, NULL, threads, NULL);

    // Wait for completion
    pthread_join(ta, NULL);
    pthread_join(tb, NULL);

    printf("Count after: %d\n", count);

    // Close the semaphore
    sem_close(mutex);
}
