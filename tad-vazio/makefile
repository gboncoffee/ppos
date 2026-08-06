# TADs filas genéricas e mapa de IDs
# Prof. Carlos A. Maziero, DINF UFPR
# Versão 2.1 - 07/2026

# ATENÇÃO: ESTE ARQUIVO NÃO DEVE SER ALTERADO;
# ALTERAÇÕES SERÃO DESCARTADAS NA CORREÇÃO.

# flags de compilação e ligação
CC     = gcc
CFLAGS = -Wall -Wextra -g -std=c99
BIN    = map-test queue-test

# estes alvos não são arquivos
.PHONY: clean purge

all: $(BIN)

# compila fila genérica
queue-test:   queue-test.o queue.o
queue-test.o: queue-test.c queue.h
queue.o:      queue.c queue.h

# compila mapa de IDs
map-test:   map-test.o map.o
map-test.o: map-test.c map.h
map.o:      map.c map.h

# limpa arquivos temporários
clean:
	-rm -f *.o *~

# limpa tudo, deixa só o o código-fonte
purge: clean
	-rm -f $(BIN)

