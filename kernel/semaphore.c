// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Semáforos e spinlocks

#include "dispatcher.h"
#include "tcb.h"

#include <map.h>
#include <queue.h>
#include <stdlib.h>

extern struct task_t* current;

struct map_t* semaphores;

struct semaphore_t {
    int             lock;
    int             value;
    struct queue_t* awaiting;
};

void spin_lock(volatile int* lock)
{
    while (__sync_fetch_and_or(lock, 1))
        ;
}

void spin_unlock(volatile int* lock)
{
    *lock = 0;
}

void sem_init()
{
    semaphores = map_create(1024);
}

void sem_term()
{
    map_destroy(semaphores);
}

int sem_create(int value)
{
    struct semaphore_t* semaphore = calloc(1, sizeof(*semaphore));
    if (semaphore == NULL)
        return -1;

    semaphore->awaiting = queue_create();
    if (semaphore->awaiting == NULL) {
        free(semaphore);
        return -1;
    }

    semaphore->value = value;

    int id = map_put(semaphores, semaphore);
    if (id == -1)
        return -1;
    return id;
}

int sem_destroy(int id)
{
    struct semaphore_t* semaphore = map_get(semaphores, id);
    if (semaphore == NULL)
        return -1;

    queue_destroy(semaphore->awaiting);
    free(semaphore);
    map_del(semaphores, id);
    return 0;
}

int sem_down(int id)
{
    volatile struct semaphore_t* semaphore =
        (volatile struct semaphore_t*) map_get(semaphores, id);
    if (semaphore == NULL)
        return -1;

    spin_lock(&semaphore->lock);

    int await = 0;
    semaphore->value -= 1;
    if (semaphore->value < 0)
        await = 1;

    spin_unlock(&semaphore->lock);

    if (await) {
        current->status = TaskStatusWaiting;
        task_suspend(semaphore->awaiting);
    }

    if (map_get(semaphores, id) == NULL)
        return -1;

    return 0;
}

int sem_up(int id)
{
    volatile struct semaphore_t* semaphore =
        (volatile struct semaphore_t*) map_get(semaphores, id);
    if (semaphore == NULL)
        return -1;

    spin_lock(&semaphore->lock);

    semaphore->value += 1;
    if (semaphore->value <= 0) {
        struct task_t* next = queue_head(semaphore->awaiting);
        if (next != NULL)
            task_awake(next);
    }

    spin_unlock(&semaphore->lock);

    return 0;
}
