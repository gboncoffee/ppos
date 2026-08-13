// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Gerência básica de tarefas.

#include "task.h"

#include <assert.h>
#include <queue.h>
#include <stdlib.h>
#include <string.h>
#include <valgrind/valgrind.h>

struct task_t   kernel_task;
struct task_t*  current;
struct queue_t* ready;

int uid;

void task_init()
{
    uid                = 0;
    kernel_task.id     = 0;
    kernel_task.parent = &kernel_task; // It created itself.
    kernel_task.name   = "kernel";
    kernel_task.status = TaskStatusRunning;

    current = &kernel_task;

    // 0 initialize so we don't accidentaly try to free the stack or something
    // like that in the future.
    memset(&kernel_task.context, 0, sizeof(kernel_task.context));

    ready = queue_create(PPOS_CONCURRENT_TASKS);
    assert(ready != NULL);

    queue_add(ready, (void*) &kernel_task);
}

void task_term()
{
}

struct task_t* task_create(char* name, void (*entry)(void*), void* arg)
{
    struct task_t* task = malloc(sizeof(*task));
    if (task == NULL)
        return NULL;

    // Calloc so we don't spill memory from other tasks. Not needed in a toy OS
    // without memory protection but it just *feels* right to.
    void* stack = calloc(PPOS_STACK_SIZE, 1);
    if (stack == NULL) {
        free(task);
        return NULL;
    }

    task->valgrind_id =
        VALGRIND_STACK_REGISTER(stack, stack + (PPOS_STACK_SIZE - 1));

    ctx_create(&task->context, entry, arg, stack, PPOS_STACK_SIZE);

    queue_add(ready, (void*) task);
    uid += 1;
    task->id     = uid;
    task->parent = current;
    task->name   = name;
    task->status = TaskStatusReady;

    return task;
}

int task_destroy(struct task_t* task)
{
    if (task == NULL)
        return ERROR;
    // TODO.
    //    if (task->status != TaskStatusFinished)
    //        return ERROR;

    if (task->context.stack != NULL)
        free(task->context.stack);

    VALGRIND_STACK_DEREGISTER(task->valgrind_id);

    queue_del(ready, task);
    free(task);

    return NOERROR;
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

    c->status    = TaskStatusWaiting;
    task->status = TaskStatusRunning;
    ctx_switch(&c->context, &task->context);

    return NOERROR;
}
