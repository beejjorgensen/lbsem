#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>

sem_t *sem;

void *thread_a(void *arg)
{
    (void)arg;

    printf("statement a1\n");

    if (sem_post(sem) == -1) {
        perror("sem_post");
    }

    return NULL;
}

void *thread_b(void *arg)
{
    (void)arg;

    if (sem_wait(sem) == -1) {
        perror("sem_wait");
    }

    printf("statement b1\n");

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
    if ((sem = sem_open_temp("sem_prog3.1", 0)) == SEM_FAILED) {
        perror("sem_open_temp");
        return 1;
    }

    // Make both threads--make them in the wrong order to encourage "b"
    // to execute first.
    pthread_create(&tb, NULL, thread_b, NULL);
    pthread_create(&ta, NULL, thread_a, NULL);

    // Wait for completion
    pthread_join(ta, NULL);
    pthread_join(tb, NULL);

    // Close the semaphore
    sem_close(sem);
}
