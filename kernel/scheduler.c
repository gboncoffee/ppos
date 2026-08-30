// GRR20235159 Gabriel Gioia de Brito
// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Escalonador de tarefas prontas.

#include "tcb.h"

#include <queue.h>

extern struct task_t* current;

void sched_init()
{
}

void sched_term()
{
}

struct task_t* scheduler(struct queue_t* ready)
{
    struct task_t* task = queue_head(ready);
    if (task == NULL)
        return NULL;

    for (struct task_t* it = task; it != NULL; it = queue_next(ready)) {
        if (it->prio < task->prio)
            task = it;
    }
    for (struct task_t* it = queue_head(ready); it != NULL;
         it                = queue_next(ready)) {
        if (it != task && it->prio > -20)
            it->prio -= 1;
    }
    task->prio = task->nice;
    return task;
}

void sched_setprio(struct task_t* task, int prio)
{
    if (task == NULL)
        task = current;
    if (prio < -20 || prio > 20)
        return;

    task->nice = prio;
    task->prio = prio;
}

int sched_getprio(struct task_t* task)
{
    // Jeito simples de indicar erros, se precisar no futuro...
    if (task == NULL)
        task = current;

    return task->nice;
}
