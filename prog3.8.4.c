#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>

sem_t *leader_queue, *follower_queue, *mutex, *rendezvous;

int leaders = 0, followers = 0;

void dance(char *name, int id)
{
    printf("%s %d: dance\n", name, id);
}

void *leader_run(void *arg)
{
    int *id = arg;

    sem_wait(mutex);

    if (followers > 0) {
        followers--;
        sem_post(follower_queue);
    } else {
        leaders++;
        sem_post(mutex);
        sem_wait(leader_queue);
    }

    dance("leader", *id);

    sem_wait(rendezvous);
    sem_post(mutex);

    return NULL;
}

void *follower_run(void *arg)
{
    int *id = arg;

    sem_wait(mutex);

    if (leaders > 0) {
        leaders--;
        sem_post(leader_queue);
    } else {
        followers++;
        sem_post(mutex);
        sem_wait(follower_queue);
    }

    dance("follower", *id);

    sem_post(rendezvous);

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
    int n = 5;
    pthread_t leader[n], follower[n];
    int leader_id[n], follower_id[n];

    // Create the semaphore
    leader_queue = sem_open_temp("sem_prog3.8.4_l", 0);
    follower_queue = sem_open_temp("sem_prog3.8.4_f", 0);
    mutex = sem_open_temp("sem_prog3.8.4_m", 1);
    rendezvous = sem_open_temp("sem_prog3.8.4_r", 0);

    // Make threads
    for (int i = 0; i < n; i++) {
        leader_id[i] = follower_id[i] = i;
        pthread_create(leader + i, NULL, leader_run, leader_id + i);
        pthread_create(follower + i, NULL, follower_run, follower_id + i);
    }

    // Wait for completion
    for (int i = 0; i < n; i++) {
        pthread_join(leader[i], NULL);
        pthread_join(follower[i], NULL);
    }

    // Close the semaphore
    sem_close(leader_queue);
    sem_close(follower_queue);
    sem_close(mutex);
    sem_close(rendezvous);
}
