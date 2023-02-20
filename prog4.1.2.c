#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>

sem_t *mutex, *items;

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
    node->next = head;

    if (head == NULL) {
        head = tail = node;
    } else {
        tail->next = node;
        tail = node;
    }
}

int get_event(void)
{
    assert(head != NULL);

    struct node *node = *head;
    int val = node->val;

    *head = (*head)->next;

    free(node);

    return val;
}

void *thread_producer(void *arg)
{
    int *tid = arg;

    for (int i = 0; i < 10; i++) {
        int event = *tid * 100 + i;
        sem_wait(mutex);
        add_event(event);
        sem_post(items);
        sem_post(mutex);
    }
}

void *thread_consumer(void *arg)
{
    int *tid = arg;

    TODO
}

int main(void)
{
    pthread_t t[n];
    int tid[n];

    // Create the semaphore
    mutex = sem_open_temp("sem_prog4.1.2_m", 1);
    items = sem_open_temp("sem_prog4.1.2_i", 0);

    TODO

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
    sem_close(items);
}
