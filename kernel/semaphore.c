// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Semáforos e spinlocks

#include "dispatcher.h"
#include "tcb.h"

#include <map.h>
#include <queue.h>
#include <stdlib.h>

extern struct task_t* current;
void                  lock_kernel();
void                  unlock_kernel();

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
    lock_kernel();

    int ret = -1;

    struct semaphore_t* semaphore = calloc(1, sizeof(*semaphore));
    if (semaphore == NULL)
        goto release;

    semaphore->awaiting = queue_create();
    if (semaphore->awaiting == NULL)
        goto free_semaphore;

    semaphore->value = value;

    ret = map_put(semaphores, semaphore);
    if (ret != -1)
        goto release;

    queue_destroy(semaphore->awaiting);
free_semaphore:
    free(semaphore);
release:
    unlock_kernel();
    return ret;
}

int sem_destroy(int id)
{
    lock_kernel();

    int ret = 0;

    struct semaphore_t* semaphore = map_get(semaphores, id);
    if (semaphore == NULL) {
        ret = -1;
        goto release;
    }

    queue_destroy(semaphore->awaiting);
    free(semaphore);
    map_del(semaphores, id);

    ret = 0;
release:
    unlock_kernel();
    return ret;
}

int sem_down(int id)
{
    lock_kernel();
    volatile struct semaphore_t* semaphore =
        (volatile struct semaphore_t*) map_get(semaphores, id);
    unlock_kernel();

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

    lock_kernel();
    volatile struct semaphore_t* s =
        (volatile struct semaphore_t*) map_get(semaphores, id);
    unlock_kernel();

    if (s == NULL)
        return -1;
    return 0;
}

int sem_up(int id)
{
    lock_kernel();
    volatile struct semaphore_t* semaphore =
        (volatile struct semaphore_t*) map_get(semaphores, id);
    unlock_kernel();

    if (semaphore == NULL)
        return -1;

    spin_lock(&semaphore->lock);

    semaphore->value += 1;
    if (semaphore->value <= 0) {
        lock_kernel();
        struct task_t* next = queue_head(semaphore->awaiting);
        if (next != NULL)
            task_awake(next);
        unlock_kernel();
    }

    spin_unlock(&semaphore->lock);

    return 0;
}
