// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Gerência de filas de mensagens

#include "dispatcher.h"
#include "semaphore.h"

#include <map.h>
#include <queue.h>
#include <stdlib.h>
#include <string.h>

void lock_kernel();
void unlock_kernel();

struct map_t* mqueues;

typedef struct {
    int             sem_buffer;
    int             sem_produce;
    int             sem_consume;
    int             msg_size;
    struct queue_t* buf;
} mqueue_t;

void mqueue_init()
{
    mqueues = map_create(1024);
}

void mqueue_term()
{
    map_destroy(mqueues);
}

int mqueue_create(int max_msgs, int msg_size)
{
    lock_kernel();
    mqueue_t* mq = calloc(1, sizeof(*mq));
    unlock_kernel();
    if (mq == NULL)
        return -1;

    lock_kernel();
    mq->buf = queue_create();
    unlock_kernel();
    if (mq->buf == NULL)
        goto free_mq;

    mq->sem_buffer = sem_create(1);
    if (mq->sem_buffer == -1)
        goto free_buf;
    mq->sem_consume = sem_create(0);
    if (mq->sem_consume == -1)
        goto destroy_sem_buffer;
    mq->sem_produce = sem_create(max_msgs);
    if (mq->sem_produce == -1)
        goto destroy_sem_consume;

    lock_kernel();
    int id = map_put(mqueues, mq);
    unlock_kernel();
    if (id >= 0)
        return id;

    sem_destroy(mq->sem_produce);
destroy_sem_consume:
    sem_destroy(mq->sem_consume);
destroy_sem_buffer:
    sem_destroy(mq->sem_buffer);
free_buf:
    lock_kernel();
    queue_destroy(mq->buf);
    unlock_kernel();
free_mq:
    lock_kernel();
    free(mq);
    unlock_kernel();
    return -1;
}

int mqueue_destroy(int id)
{
    lock_kernel();
    mqueue_t* mq = (mqueue_t*) map_get(mqueues, id);
    unlock_kernel();
    if (mq == NULL)
        return -1;

    lock_kernel();
    map_del(mqueues, id);
    for (void* buffer = queue_head(mq->buf); buffer != NULL;
         buffer       = queue_next(mq->buf)) {
        free(buffer);
    }
    queue_destroy(mq->buf);
    unlock_kernel();

    sem_destroy(mq->sem_buffer);
    sem_destroy(mq->sem_consume);
    sem_destroy(mq->sem_produce);

    lock_kernel();
    free(mq);
    unlock_kernel();

    return 0;
}

int mqueue_send(int id, void* msg)
{
    int ret = -1;

    lock_kernel();
    mqueue_t* mq = (mqueue_t*) map_get(mqueues, id);
    if (mq == NULL) {
        unlock_kernel();
        return -1;
    }

    int prod = mq->sem_produce;
    int buf  = mq->sem_buffer;
    unlock_kernel();

    if (sem_down(prod) == -1)
        return -1;
    if (sem_down(buf) == -1)
        return -1;

    lock_kernel();
    mq = (mqueue_t*) map_get(mqueues, id);
    if (mq == NULL)
        goto release;
    char* msg_on_queue = calloc(1, mq->msg_size);
    if (msg != NULL) {
        memcpy(msg_on_queue, msg, mq->msg_size);
        queue_add(mq->buf, msg_on_queue);
        ret = 0;
    }

release:
    unlock_kernel();
    sem_up(mq->sem_buffer);
    sem_up(mq->sem_consume);

    return ret;
}

int mqueue_recv(int id, void* msg)
{
    int ret = -1;

    lock_kernel();
    mqueue_t* mq = (mqueue_t*) map_get(mqueues, id);
    if (mq == NULL) {
        unlock_kernel();
        return -1;
    }

    int cons = mq->sem_consume;
    int buf  = mq->sem_buffer;

    unlock_kernel();

    if (sem_down(cons) == -1)
        return -1;
    if (sem_down(buf) == -1)
        return -1;

    lock_kernel();
    mq = (mqueue_t*) map_get(mqueues, id);
    if (mq == NULL)
        goto release;
    char* buffer = queue_head(mq->buf);
    queue_del(mq->buf, buffer);
    if (buffer != NULL) {
        memcpy(msg, buffer, mq->msg_size);
        free(buffer);
        ret = 0;
    }

release:
    unlock_kernel();
    sem_up(mq->sem_buffer);
    sem_up(mq->sem_produce);

    return ret;
}
