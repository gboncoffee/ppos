// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Dispatcher: gerencia os estados das tarefas.

#include "dispatcher.h"

#include "scheduler.h"
#include "task.h"

#include <assert.h>
#include <queue.h>
#include <stdio.h>

struct queue_t* ready;

extern struct task_t* current;
extern struct task_t  kernel_task;

void dispatcher_init()
{
    ready = queue_create();
}

void dispatcher_term()
{
    queue_destroy(ready);
}

void user_main(void*);

void dispatcher()
{
    task_create("user_main", user_main, NULL);

    while (queue_size(ready) > 0) {
        struct task_t* task = scheduler(ready);

        if (task != NULL)
            task_run(task);

        if (task->status == TaskStatusFinished) {
            task_destroy(task);
        }
    }
}

void task_run(struct task_t* task)
{
    assert(queue_del(ready, task) == NOERROR);

    task->status = TaskStatusRunning;
    task_switch(task);
}

void task_yield()
{
    current->status = TaskStatusReady;
    queue_add(ready, current);
    task_switch(&kernel_task);
}

void task_suspend(struct queue_t* queue)
{
    current->status = TaskStatusWaiting;

    if (queue != NULL) {
        current->waiting_queue = queue;
        queue_add(queue, current);
    }

    task_switch(&kernel_task);
}

void task_awake(struct task_t* task)
{
    if (task->waiting_queue != NULL) {
        queue_del(task->waiting_queue, task);
        task->waiting_queue = NULL;
    }

    task->status = TaskStatusReady;
    queue_add(ready, task);
}

void task_exit(int exit_code)
{
    current->status = TaskStatusFinished;
    task_switch(&kernel_task);
}
