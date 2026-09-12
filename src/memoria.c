#include <stdio.h>
#include <stdint.h>
#include "memoria.h"

void agregar_byte(Memoria * mem, uint8_t dato){
    // Validar memoria
    mem->mem_principal[0] = dato;
}
