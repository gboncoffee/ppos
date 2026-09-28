// GRR20235159 Gabriel Gioia de Brito
// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Gerência básica de tarefas.

#include "task.h"
#include "time.h"

#include <assert.h>
#include <queue.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <valgrind/valgrind.h>

struct task_t          kernel_task;
struct task_t*         current;
extern struct queue_t* ready;
void                   lock_kernel();
void                   unlock_kernel();

int uid;

void task_init()
{
    uid                   = 0;
    kernel_task.id        = 0;
    kernel_task.parent    = &kernel_task; // It created itself.
    kernel_task.name      = "kernel";
    kernel_task.status    = TaskStatusRunning;
    kernel_task.is_system = true;

    current = &kernel_task;

    // 0 initialize so we don't accidentaly try to free the stack or something
    // like that in the future.
    memset(&kernel_task.context, 0, sizeof(kernel_task.context));
}

void task_term()
{
}

struct task_t* task_create(char* name, void (*entry)(void*), void* arg)
{
    lock_kernel();

    struct task_t* task = calloc(1, sizeof(*task));
    if (task == NULL)
        goto release;

    // Calloc so we don't spill memory from other tasks. Not needed in a toy OS
    // without memory protection but it just *feels* right to.
    void* stack = calloc(PPOS_STACK_SIZE, 1);
    if (stack == NULL) {
        free(task);
        goto release;
    }

    task->valgrind_id =
        VALGRIND_STACK_REGISTER(stack, stack + (PPOS_STACK_SIZE - 1));

    ctx_create(&task->context, entry, arg, stack, PPOS_STACK_SIZE);

    uid += 1;
    task->id               = uid;
    task->parent           = current;
    task->name             = name;
    task->status           = TaskStatusReady;
    task->waiting_on_queue = queue_create();
    task->wall_start       = time();

    queue_add(ready, task);

release:
    unlock_kernel();
    return task;
}

int task_destroy(struct task_t* task)
{
    lock_kernel();

    int ret = NOERROR;
    if (task == NULL || task->status != TaskStatusFinished) {
        ret = ERROR;
        goto release;
    }

    if (task->context.stack != NULL)
        free(task->context.stack);

    VALGRIND_STACK_DEREGISTER(task->valgrind_id);

    free(task->waiting_on_queue);
    free(task);

release:
    unlock_kernel();
    return ret;
}

int task_id(struct task_t* task)
{
    if (task == NULL)
        task = current;

    return task->id;
}

char* task_name(struct task_t* task)
{
    if (task == NULL)
        task = current;

    return task->name;
}

int task_switch(struct task_t* task)
{
    if (task == NULL)
        task = current->parent;
    if (task == NULL)
        return ERROR;

    struct task_t* c = current;
    current          = task;

    ctx_switch(&c->context, &task->context);

    return NOERROR;
}
