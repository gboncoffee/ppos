// PingPongOS - PingPong Operating System
// © Prof. Carlos A. Maziero, DINF UFPR
// Versão 2.1 -- 06/2026

// Este arquivo PODE/DEVE ser alterado.

// Descritor de tarefas (TCB - Task Control Block).

#ifndef __PPOS_TCB__
#define __PPOS_TCB__

#include "ctx.h"

#include <queue.h>

#define PPOS_STACK_SIZE (4096 * 16)

typedef enum {
    TaskStatusReady,
    TaskStatusRunning,
    TaskStatusWaiting,
    TaskStatusFinished
} TaskStatus;

// Task Control Block (TCB), infos sobre uma tarefa
struct task_t {
    struct ctx_t    context; // contexto da tarefa
    char*           name; // nome da tarefa
    struct task_t*  parent;
    struct queue_t* waiting_queue;
    int             id; // identificador da tarefa
    int             valgrind_id;
    TaskStatus      status;
};

#define PPOS_CONCURRENT_TASKS (1024)

#endif
