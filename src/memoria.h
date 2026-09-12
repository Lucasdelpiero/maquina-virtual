#ifndef HEADER_42C6CCD3C6A65309
#define HEADER_42C6CCD3C6A65309


#include <stdio.h>
#include <stdint.h>

#define TAM_MEMORIA_PRINCIPAL 50 // Van a ser 16KiB
#define TAM_SEGMENTOS 16

typedef struct {
    uint32_t tabla_segmentos[TAM_SEGMENTOS];
    uint8_t mem_principal[TAM_MEMORIA_PRINCIPAL];
} Memoria;

// Usado solo en el inicializador
void inicializar_memoria(Memoria *mem);
void escribir_byte(Memoria *mem, int pos,uint8_t dato);
int32_t get_dir_fisica(Memoria mem, int32_t dir_logica);
void set_valor_memoria(Memoria *mem, int32_t dir_logica, int32_t dato, int cant_accedidos);
int32_t get_valor_memoria(Memoria memoria, int32_t dir_logica,  int cant_accedidos);
void set_valor_operando(Memoria *mem, int32_t operando, int32_t dato, int cant_accedidos);
uint32_t get_valor_operando(Memoria mem, int32_t operando, int cant_accedidos);


#endif // header guard
