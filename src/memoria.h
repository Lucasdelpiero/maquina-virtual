#ifndef MEMORIA_H
#define MEMORIA_H

#include <stdio.h>
#include <stdint.h>

#define TAM_MEMORIA_PRINCIPAL 16384 // Van a ser 16KiB
#define TAM_SEGMENTOS 8

typedef struct {
    uint32_t tabla_segmentos[TAM_SEGMENTOS];
    uint8_t mem_principal[TAM_MEMORIA_PRINCIPAL];
    uint32_t *registros;
} Memoria;

// Funciones de manipulacion de memoria
void inicializar_memoria(Memoria *mem);
void escribir_byte(Memoria *mem, int pos, uint8_t dato);
int32_t get_dir_fisica(Memoria *mem, int32_t dir_logica);
void set_registros_memoria(Memoria *mem, int32_t dir_logica, int cant_accedidos, int32_t dir_fisica, int32_t valor);
int32_t get_valor_memoria(Memoria *mem, int32_t dir_logica, int cant_accedidos);
int32_t set_valor_memoria(Memoria *mem, int32_t dir_logica, int32_t dato, int cant_accedidos);

// Interfaz de memoria de la maquina virtual
uint16_t traducir_direccion(Memoria *mem, int32_t dir_logica, uint16_t cant_bytes);
int32_t leer_memoria(Memoria *mem, uint16_t dir_fisica, uint8_t cant_bytes);
void escribir_memoria(Memoria *mem, uint16_t dir_fisica, uint8_t cant_bytes, int32_t valor);

#endif // MEMORIA_H
