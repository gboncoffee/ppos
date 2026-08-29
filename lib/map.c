// GRR20235159 Gabriel Gioia de Brito
// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 2.1 -- 07/2026

// Este arquivo PODE/DEVE ser alterado.

// Implementação do TAD Mapa de objetos

#include "map.h"

#include <stdio.h>
#include <stdlib.h>

struct map_t {
    int   size;
    void* items[];
};

struct map_t* map_create(int size)
{
    struct map_t* map = calloc(1, sizeof(*map) + sizeof(void*) * size);
    map->size         = size;
    return map;
}

int map_destroy(struct map_t* map)
{
    if (map == NULL)
        return ERROR;

    free(map);

    return NOERROR;
}

int map_put(struct map_t* map, void* object)
{
    if (map == NULL)
        return ERROR;

    for (int i = 0; i < map->size; i += 1) {
        if (map->items[i] == NULL) {
            map->items[i] = object;
            return i;
        }
    }

    return ERROR;
}

void* map_get(struct map_t* map, int id)
{
    if (map == NULL || id >= map->size)
        return NULL;
    return map->items[id];
}

void* map_del(struct map_t* map, int id)
{
    if (map == NULL || id >= map->size)
        return NULL;

    void* p        = map->items[id];
    map->items[id] = NULL;
    return p;
}

int map_items(struct map_t* map)
{
    if (map == NULL)
        return ERROR;

    int c = 0;
    for (int i = 0; i < map->size; i += 1)
        c += map->items[i] != NULL;

    return c;
}

int map_size(struct map_t* map)
{
    if (map == NULL)
        return ERROR;
    return map->size;
}

void map_print(char* name, struct map_t* map)
{
    printf("%s: ", name);
    if (map == NULL) {
        printf("undef\n");
        return;
    }
    printf("[ ");

    int c = 0;
    for (int i = 0; i < map->size; i += 1) {
        if (map->items[i] == NULL) {
            printf("- ");
        } else {
            printf("* ");
            c += 1;
        }
    }
    printf("] (%d/%d)\n", c, map->size);
}
