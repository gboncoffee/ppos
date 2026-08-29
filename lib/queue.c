// GRR20235159 Gabriel Gioia de Brito
// PingPongOS - PingPong Operating System
// © Prof. Carlos A. Maziero, DINF UFPR
// Versão 2.1 -- 06/2026

// Este arquivo PODE/DEVE ser alterado.

// Implementação do TAD fila genérica

#include "queue.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct node_t {
    struct node_t* next;
    void*          item;
} node_t;

struct queue_t {
    node_t* head;
    node_t* tail;
    node_t* it;
};

struct queue_t* queue_create()
{
    return (struct queue_t*) calloc(1, sizeof(struct queue_t));
}

int queue_destroy(struct queue_t* queue)
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

int queue_add(struct queue_t* queue, void* item)
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
        queue->tail       = new;
    }

    return NOERROR;
}

int queue_del(struct queue_t* queue, void* item)
{
    if (queue == NULL)
        return ERROR;
    // Works because of the layout of the structs.
    struct node_t* parent = (struct node_t*) queue;
    struct node_t* node   = queue->head;
    while (node != NULL) {
        if (node->item == item) {
            parent->next = node->next;
            if (queue->it == node)
                queue->it = node->next;
            if (queue->tail == node) {
                if (parent == (struct node_t*) queue) {
                    queue->head = NULL;
                    queue->tail = NULL;
                } else {
                    queue->tail = parent;
                }
            }
            free(node);
            return NOERROR;
        }
        parent = node;
        node   = node->next;
    }

    return ERROR;
}

bool queue_has(struct queue_t* queue, void* item)
{
    for (node_t* node = queue->head; node != NULL; node = node->next) {
        if (node->item == item)
            return true;
    }
    return false;
}

int queue_size(struct queue_t* queue)
{
    if (queue == NULL)
        return ERROR;

    int n = 0;
    for (node_t* node = queue->head; node != NULL; node = node->next)
        n += 1;
    return n;
}

void* queue_head(struct queue_t* queue)
{
    if (queue == NULL)
        return NULL;
    queue->it = queue->head;
    if (queue->head == NULL)
        return NULL;
    return queue->head->item;
}

void* queue_next(struct queue_t* queue)
{
    if (queue == NULL || queue->it == NULL)
        return NULL;

    queue->it = queue->it->next;
    if (queue->it == NULL)
        return NULL;

    return queue->it->item;
}

void* queue_item(struct queue_t* queue)
{
    if (queue == NULL || queue->it == NULL)
        return NULL;
    return queue->it->item;
}

void queue_print(char* name, struct queue_t* queue, void(func)(void*))
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
        if (func != NULL)
            func(node->item);
        else
            printf("undef");
        printf(" ");
    }

    printf("] (%d items)\n", i);
}
