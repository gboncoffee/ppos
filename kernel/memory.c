// GRR20235159 Gabriel Gioia de Brito
// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Alocador básico de memória heap.

#include "ppos.h"

#include <stdint.h>
#include <string.h>

void lock_kernel(void);
void unlock_kernel(void);

#define HEAP_BYTES (64u * 1024u * 1024u)
#define BLOCK_ALIGNMENT 16u

typedef struct block_t {
    unsigned int size;
    unsigned int used;
    unsigned int reserved[2];
} block_t;

typedef char block_header_must_be_aligned
    [sizeof(block_t) % BLOCK_ALIGNMENT == 0 ? 1 : -1];

static unsigned char heap[HEAP_BYTES] __attribute__((aligned(BLOCK_ALIGNMENT)));

static unsigned char* heap_end(void)
{
    return heap + HEAP_BYTES;
}

static block_t* next_block(block_t* block)
{
    return (block_t*) ((unsigned char*) (block + 1) + block->size);
}

static int valid_block(block_t* block)
{
    unsigned char* header  = (unsigned char*) block;
    unsigned char* payload = (unsigned char*) (block + 1);

    if (header < heap || payload > heap_end() || block->size == 0)
        return 0;

    if (block->size % BLOCK_ALIGNMENT != 0)
        return 0;

    return block->size <= (unsigned int) (heap_end() - payload);
}

static unsigned int aligned_size(unsigned int size)
{
    return (size + BLOCK_ALIGNMENT - 1) & ~(BLOCK_ALIGNMENT - 1);
}

void mem_init(void)
{
    memset(heap, 0, sizeof(heap));

    block_t* block = (block_t*) heap;
    block->size    = HEAP_BYTES - sizeof(*block);
    block->used    = 0;
}

void mem_term(void)
{
}

void* mem_alloc(int size)
{
    if (size <= 0 || (unsigned int) size > HEAP_BYTES - sizeof(block_t))
        return NULL;

    unsigned int wanted = aligned_size((unsigned int) size);
    if (wanted < (unsigned int) size)
        return NULL;

    lock_kernel();

    block_t* block = (block_t*) heap;
    while ((unsigned char*) block < heap_end()) {
        if (!valid_block(block)) {
            printk("heap: corrupted block at %p\n", block);
            unlock_kernel();
            return NULL;
        }

        if (!block->used && block->size >= wanted)
            break;

        block = next_block(block);
    }

    if ((unsigned char*) block >= heap_end()) {
        unlock_kernel();
        return NULL;
    }

    unsigned int remaining = block->size - wanted;
    if (remaining >= sizeof(block_t) + BLOCK_ALIGNMENT) {
        block_t* new = (block_t*) ((unsigned char*) (block + 1) + wanted);
        new->size    = remaining - sizeof(*new);
        new->used    = 0;
        block->size  = wanted;
    }

    block->used = 1;
    void* addr  = block + 1;

    unlock_kernel();
    return addr;
}

int mem_free(void* addr)
{
    if (addr == NULL || (unsigned char*) addr < heap + sizeof(block_t)
        || (unsigned char*) addr >= heap_end()
        || (uintptr_t) addr % BLOCK_ALIGNMENT != 0)
        return ERROR;

    lock_kernel();

    block_t* block = (block_t*) heap;
    block_t* prev  = NULL;
    while ((unsigned char*) block < heap_end()) {
        if (!valid_block(block)) {
            printk("heap: corrupted block at %p\n", block);
            unlock_kernel();
            return ERROR;
        }

        if ((void*) (block + 1) == addr)
            break;

        prev  = block;
        block = next_block(block);
    }

    if ((unsigned char*) block >= heap_end() || !block->used) {
        unlock_kernel();
        return ERROR;
    }

    block->used = 0;

    block_t* next = next_block(block);
    if ((unsigned char*) next < heap_end() && valid_block(next) && !next->used)
        block->size += sizeof(*next) + next->size;

    if (prev != NULL && !prev->used)
        prev->size += sizeof(*block) + block->size;

    unlock_kernel();
    return NOERROR;
}

int mem_size(void)
{
    return HEAP_BYTES;
}

int mem_avail(void)
{
    lock_kernel();

    unsigned int size = 0;
    block_t* block    = (block_t*) heap;
    while ((unsigned char*) block < heap_end()) {
        if (!valid_block(block)) {
            printk("heap: corrupted block at %p\n", block);
            break;
        }

        if (!block->used)
            size += block->size;

        block = next_block(block);
    }

    unlock_kernel();
    return (int) size;
}

void mem_report(void)
{
    lock_kernel();

    unsigned int used = 0;
    unsigned int free = 0;
    int nblks         = 0;
    int nnblks        = 0;
    int total_blocks  = 0;
    int corrupted     = 0;

    block_t* block = (block_t*) heap;
    while ((unsigned char*) block < heap_end()) {
        if (!valid_block(block)) {
            printk("heap: corrupted block at %p\n", block);
            corrupted = 1;
            break;
        }

        if (block->used) {
            used += block->size;
            nblks++;
        } else {
            free += block->size;
            nnblks++;
        }

        total_blocks++;
        block = next_block(block);
    }

    printk(
        "heap: %d KB allocated (%d blocks), %d KB free (%d blocks)\n",
        used / 1000,
        nblks,
        free / 1000,
        nnblks
    );

    if (!corrupted) {
        int i = 0;
        block = (block_t*) heap;
        while ((unsigned char*) block < heap_end()) {
            unsigned int total_size = sizeof(*block) + block->size;
            printk(
                "heap: block %5d: %p - %p %s prev %5d next %5d size %9d\n",
                i,
                block,
                (unsigned char*) block + total_size - 1,
                block->used ? "aloc" : "FREE",
                i > 0 ? i - 1 : total_blocks - 1,
                i + 1 < total_blocks ? i + 1 : 0,
                total_size
            );
            i++;
            block = next_block(block);
        }
    }

    unlock_kernel();
}
