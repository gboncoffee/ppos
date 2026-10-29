// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Gerência de um dispositivo orientado a blocos.

#include "dispatcher.h"
#include "mqueue.h"
#include "task.h"

#include <hardware/cpu.h>
#include <hardware/disk.h>
#include <queue.h>

typedef enum {
    RequestRead,
    RequestWrite,
} RequestType;

typedef struct {
    struct task_t* task;
    void*          block;
    int            addr;
    RequestType    type;
} request_t;

int number_of_blocks;
int blk_size;

struct queue_t* awaiting;
struct queue_t* driver_suspended;
int             requests;

struct task_t* disk_driver_task;

extern struct task_t* current;

request_t cur;

void lock_kernel(void);
void unlock_kernel(void);

void disk_irq_handler(int arg)
{
    (void) arg;
    task_awake(disk_driver_task);
}

void disk_driver(void* arg)
{
    for (;;) {
        if (mqueue_recv(requests, &cur) != NOERROR)
            task_exit(1);

        int cmd = cur.type == RequestRead ? DISK_CMD_READ : DISK_CMD_WRITE;

        hw_disk(cmd, cur.addr, cur.block);
        task_suspend(driver_suspended);
        task_awake(cur.task);
    }
}

void block_init(char* disk_image)
{
    if (hw_disk(DISK_CMD_INIT, 0, disk_image))
        return;

    requests = mqueue_create(1, sizeof(request_t));
    awaiting = queue_create();

    number_of_blocks = hw_disk(DISK_CMD_DISKSIZE, 0, 0);
    blk_size         = hw_disk(DISK_CMD_BLOCKSIZE, 0, 0);

    disk_driver_task            = task_create("disk_driver", disk_driver, NULL);
    disk_driver_task->is_system = 1;

    hw_irq_handle(IRQ_DISK, disk_irq_handler);
}

void block_term(char* disk_image)
{
    mqueue_destroy(requests);
    queue_destroy(awaiting);

    task_destroy(disk_driver_task);
}

int block_size(void)
{
    return blk_size;
}

int block_blocks(void)
{
    return number_of_blocks;
}

int block_read(int block, void* buffer)
{
    request_t msg;
    msg.task  = current;
    msg.block = buffer;
    msg.addr  = block;
    msg.type  = RequestRead;
    mqueue_send(requests, (void*) &msg);
    task_suspend(awaiting);
    return 0;
}

int block_write(int block, void* buffer)
{
    request_t msg;
    msg.task  = current;
    msg.block = buffer;
    msg.addr  = block;
    msg.type  = RequestWrite;
    mqueue_send(requests, (void*) &msg);
    task_suspend(awaiting);
    return 0;
}
