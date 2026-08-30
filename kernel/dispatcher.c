// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Dispatcher: gerencia os estados das tarefas.

#include "dispatcher.h"

#include "scheduler.h"
#include "task.h"
#include "time.h"

#include <assert.h>
#include <pplibc.h>
#include <queue.h>

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
    kernel_task.wall_start     = time();
    kernel_task.wall_last_grab = kernel_task.wall_start;
    kernel_task.activations    = 1;

    struct task_t* user_task = task_create("user_main", user_main, NULL);

    while (queue_size(ready) > 0) {
        struct task_t* task = scheduler(ready);

        if (task != NULL) {
            kernel_task.cpu_time += time() - kernel_task.wall_last_grab;
            task_run(task);
        }

        kernel_task.activations += 1;
        if (task->status == TaskStatusFinished) {
            task->wall_last_grab = time() - task->wall_start;
            printk(
                "PPOS: task %d (%s) %d ms run, %d ms cpu, %d acts, exit "
                "code %d\n",
                task->id,
                task->name,
                task->wall_last_grab,
                task->cpu_time,
                task->activations,
                task->exit_code
            );
            struct queue_t* q = task->waiting_on_queue;
            for (struct task_t* wt = queue_head(q); wt != NULL;
                 wt                = queue_next(q)) {
                task_awake(wt);
            }
        }
    }

    task_destroy(user_task);

    kernel_task.cpu_time += time() - kernel_task.wall_last_grab;
    printk(
        "PPOS: task %d (%s) %d ms run, %d ms cpu, %d acts, exit "
        "code %d\n",
        kernel_task.id,
        kernel_task.name,
        time() - kernel_task.wall_start,
        kernel_task.cpu_time,
        kernel_task.activations,
        0
    );
}

void task_run(struct task_t* task)
{
    assert(queue_del(ready, task) == NOERROR);

    task->status  = TaskStatusRunning;
    task->quantum = 10;
    task->activations += 1;
    task->wall_last_grab = time();
    if (task->wall_start == 0)
        task->wall_start = task->wall_last_grab;
    task_switch(task);
}

void task_yield()
{
    current->status = TaskStatusReady;
    current->cpu_time += time() - current->wall_last_grab;

    queue_add(ready, current);

    kernel_task.wall_last_grab = time();
    task_switch(&kernel_task);
}

void task_suspend(struct queue_t* queue)
{
    current->status = TaskStatusWaiting;
    current->cpu_time += time() - current->wall_last_grab;

    if (queue != NULL) {
        current->waiting_queue = queue;
        queue_add(queue, current);
    }

    kernel_task.wall_last_grab = time();
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
    current->status    = TaskStatusFinished;
    current->exit_code = exit_code;

    current->cpu_time += time() - current->wall_last_grab;

    kernel_task.wall_last_grab = time();
    task_switch(&kernel_task);
}

int task_wait(struct task_t* task)
{
    if (task == NULL)
        return ERROR;
    if (task->status == TaskStatusFinished)
        return task->exit_code;
    task_suspend(task->waiting_on_queue);
    return task->exit_code;
}
