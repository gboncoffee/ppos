// GRR20235159 Gabriel Gioia de Brito
// PingPongOS - PingPong Operating System
// © Prof. Carlos A. Maziero, DINF UFPR
// Versão 2.1 -- 06/2026

// Este arquivo PODE/DEVE ser alterado.

// Descritor de tarefas (TCB - Task Control Block).

#ifndef __PPOS_TCB__
#define __PPOS_TCB__

#include "ctx.h"

#include <queue.h>
#include <stdbool.h>

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
    struct queue_t* waiting_on_queue;
    unsigned long   wall_start; // wall time de quando ela começou
    // wall time da última vez que ela ganhou a cpu, ou o wall time total caso
    // ela já tenha finalizado
    unsigned long wall_last_grab;
    unsigned long cpu_time; // tempo em que ela passou com a cpu
    unsigned long activations;
    int           id; // identificador da tarefa
    int           valgrind_id;
    int           exit_code;
    int           nice;
    int           prio;
    int           quantum;
    TaskStatus    status;
    bool          is_system;
};

#define PPOS_CONCURRENT_TASKS (1024)

#endif
