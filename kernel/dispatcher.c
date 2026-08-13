// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Dispatcher: gerencia os estados das tarefas.

#include "dispatcher.h"

#include "task.h"

void dispatcher_init()
{
}

void dispatcher_term()
{
}

void user_main(void*);

void dispatcher()
{
    struct task_t* task_user = task_create("user_main", user_main, NULL);
    task_switch(task_user);
    task_destroy(task_user);
}
