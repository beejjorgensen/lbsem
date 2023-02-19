#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>

sem_t *a_arrived, *b_arrived;

void *thread_a(void *arg)
{
    (void)arg;

    printf("statement a1\n");

    if (sem_post(a_arrived) == -1) {
        perror("A: sem_post");
    }

    if (sem_wait(b_arrived) == -1) {
        perror("A: sem_wait");
    }

    printf("statement a2\n");

    return NULL;
}

void *thread_b(void *arg)
{
    (void)arg;

    printf("statement b1\n");

    if (sem_post(b_arrived) == -1) {
        perror("B: sem_post");
    }

    if (sem_wait(a_arrived) == -1) {
        perror("B: sem_wait");
    }

    printf("statement b2\n");

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
    if ((a_arrived = sem_open_temp("sem_prog3.3_a", 0)) == SEM_FAILED) {
        perror("sem_open_temp");
        return 1;
    }

    if ((b_arrived = sem_open_temp("sem_prog3.3_b", 0)) == SEM_FAILED) {
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
    sem_close(a_arrived);
    sem_close(b_arrived);
}
