#ifndef HEADER_42C6CCD3C6A65309
#define HEADER_42C6CCD3C6A65309
#endif // header guard

#include <stdio.h>
#include <stdint.h>

#define TAM_MEMORIA_PRINCIPAL 160 // Van a ser 16KiB
#define TAM_SEGMENTOS 16

typedef struct {
    uint32_t tabla_segmentos[TAM_SEGMENTOS];
    uint8_t mem_principal[TAM_MEMORIA_PRINCIPAL];
    uint32_t ultima_pos;
} Memoria;

// Usado solo en el inicializador
void agregar_byte(Memoria *mem, uint8_t dato);
