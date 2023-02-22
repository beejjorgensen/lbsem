#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>

sem_t *mutex, *items;

const int pn = 20, cn = 50;  // Number of threads
const int production_count = 10;

int sem_init(sem_t *sem, int pshared, unsigned int value)
{
    (void)pshared;  // Always effectively `0`
    const char const *name = "m

    // Create the semaphore
    if ((*sem = sem_open(name, O_CREAT, 0600, value)) == SEM_FAILED)
        return -1;

    // Unlink it so it will go away after this process exits
    if (sem_unlink(name) == -1) {
        sem_close(sem);
        return -1;
    }

    return 0;
}

sem_t *sem_open_temp(const char *name, int value)
{
    sem_t *sem;

    // Create the semaphore
    if ((sem = sem_open(name, O_CREAT, 0600, value)) == SEM_FAILED)
        return SEM_FAILED;

    // Unlink it so it will go away after this process exits
    if (sem_unlink(name) == -1) {
        sem_close(sem);
        return SEM_FAILED;
    }

    return sem;
}

struct node {
    int val;
    struct node *next;
};

struct node *head = NULL, *tail;

void add_event(int val)
{
    struct node *node = malloc(sizeof *node);

    assert(node);

    node->val = val;
    node->next = NULL;

    if (head == NULL) {
        head = tail = node;
    } else {
        tail->next = node;
        tail = node;
    }
}

int get_event(void)
{
    if (head == NULL) return -999;

    struct node *node = head;
    int val = node->val;

    head = head->next;

    free(node);

    return val;
}

void *thread_producer(void *arg)
{
    int *tid = arg;

    for (int i = 0; i < production_count; i++) {
        int event = *tid * 100 + i;
        //usleep(0.5*1000*1000);
        sem_wait(mutex);
        add_event(event);
        sem_post(mutex);
        sem_post(items);
        printf("P%d: added event %d\n", *tid, event);
    }

    return NULL;
}

void *thread_consumer(void *arg)
{
    int *tid = arg;
    int done = 0;

    while (!done) {
        sem_wait(items);
        sem_wait(mutex);
        int event = get_event();
        sem_post(mutex);
        printf("C%d: got event %d\n", *tid, event);
        if (event == -999) done = 1;
    }

    return NULL;
}

int main(void)
{
    pthread_t pt[pn], ct[cn];
    int ptid[pn], ctid[cn];

    // Create the semaphore
    mutex = sem_open_temp("sem_prog4.1.2_m", 1);
    items = sem_open_temp("sem_prog4.1.2_i", 0);

    // Create producer threads
    for (int i = 0; i < pn; i++) {
        ptid[i] = i;
        pthread_create(pt + i, NULL, thread_producer, ptid + i);
    }

    // Create consumer threads
    for (int i = 0; i < cn; i++) {
        ctid[i] = i;
        pthread_create(ct + i, NULL, thread_consumer, ctid + i);
    }

    // Wait for completion
    for (int i = 0; i < pn; i++)
        pthread_join(pt[i], NULL);

    // After producers are done, post to wake up the consumers
    for (int i = 0; i < cn; i++)
        sem_post(items);

    // Wait for completion
    for (int i = 0; i < cn; i++)
        pthread_join(ct[i], NULL);

    // Close the semaphore
    sem_close(mutex);
    sem_close(items);
}
