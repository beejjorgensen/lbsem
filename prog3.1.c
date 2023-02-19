#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define SEM_NAME "sem_prog3.1"

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

int main(void)
{
    pthread_t ta, tb;

    // Create the semaphore
    if ((sem = sem_open(SEM_NAME, O_CREAT, 0666, 0)) == SEM_FAILED) {
        perror("sem_open");
        return 1;
    }

    // Unlink it so it will go away after this process exits
    if (sem_unlink(SEM_NAME) == -1) {
        perror("sem_unlink");
        return 2;
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
