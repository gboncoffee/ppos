// PingPongOS - PingPong Operating System
// © Prof. Carlos A. Maziero, DINF UFPR
// Versão 2.1 -- 06/2026

// Este arquivo PODE/DEVE ser alterado.

// Implementação do TAD fila genérica

#include "queue.h"

#include <stdlib.h>

typedef struct node_t {
    node_t* next;
    void*   item;
} node_t;

typedef struct {
    node_t* head;
    node_t* tail;
    node_t* it;
} queue_t;

queue_t* queue_create()
{
    return (queue_t*) calloc(1, sizeof(*queue));
}

int queue_destroy(queue_t* queue)
{
    if (queue == NULL)
        return ERROR;

    node_t* node = queue->head;
    while (node != NULL) {
        node_t* n = node;
        node      = node->next;
        free(n);
    }

    free(queue);

    return NOERROR;
}

int queue_add(queue_t* queue, void* item)
{
    if (queue == NULL)
        return ERROR;

    node_t* new = malloc(sizeof(*new));
    new->item   = item;
    new->next   = NULL;

    if (queue->tail == NULL) {
        queue->tail = new;
        queue->head = new;
        queue->it   = new;
    } else {
        queue->tail->next = new;
    }

    return NOERROR;
}

int queue_del(queue_t* queue, void* item)
{
    node_t** prev = &queue->head;
    node_t*  node = queue->head;
    while (node != NULL) {
        if (node->item == item) {
            *prev = node->next;
            free(node);
            return NOERROR;
        }
        prev = &node->next;
        node = node->next;
    }

    return ERROR;
}

bool queue_has(queue_t* queue, void* item)
{
    for (node_t* node = queue->head; node != NULL; node = node->next) {
        if (node->item == item)
            return true;
    }
    return false;
}

int queue_size(queue_t* queue)
{
    int n = 0;
    for (node_t* node = queue->head; node != NULL; node = node->next)
        n += 1;
    return n;
}

void* queue_head(queue_t* queue)
{
    return queue->head;
}

void* queue_next(queue_t* queue)
{
    if (queue->it == NULL)
        return NULL;

    void* item = queue->it->item;

    queue->it = queue->it->next;
    if (queue->it == NULL)
        queue->it = queue->head;

    return item;
}

void* queue_item(queue_t* queue)
{
    if (queue->it == NULL)
        return NULL;
    return queue->it->item;
}

void queue_print(char* name, queue_t* queue, void(func)(void*))
{
    printf("%s: ", name);
    if (queue == NULL) {
        printf("undef\n");
        return;
    }
    printf("[ ");

    int i = 0;
    for (node_t* node = queue->head; node != NULL; node = node->next) {
        i += 1;
        func(node->item);
        printf(" ");
    }

    printf("] (%d ite%s)\n", i, i == 1 ? "em" : "ns");
}
