// GRR20235159 Gabriel Gioia de Brito
// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Gerência básica do tempo.

#include "dispatcher.h"
#include "hardware/cpu.h"
#include "task.h"
#include "tcb.h"

unsigned long         ctime;
extern struct task_t* current;
extern struct task_t  kernel_task;

void tick(int arg)
{
    (void) arg;
    ctime += 1;

    if (current == NULL)
        return;
    if (current->is_system)
        return;

    current->quantum -= 1;
    if (current->quantum == 0)
        task_yield();
}

void time_init()
{
    ctime = 0;
    hw_irq_handle(IRQ_TIMER, tick);
    hw_timer(1, 1);
}

void time_term()
{
    hw_timer(0, 0);
}

unsigned int time()
{
    return ctime;
}
