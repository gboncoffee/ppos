// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Escalonador de tarefas prontas.

#include "tcb.h"

#include <queue.h>

void sched_init()
{
}

void sched_term()
{
}

struct task_t* scheduler(struct queue_t* ready)
{
    return queue_head(ready);
}
