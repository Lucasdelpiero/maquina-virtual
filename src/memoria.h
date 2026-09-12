#ifndef MEMORIA_H
#define MEMORIA_H

#include "vmx.h"

// Funciones del subsistema de memoria (Etapa 3 - a implementar por Lucas)
uint16_t traducir_direccion(Vmx *vmx, int32_t dir_logica, uint16_t cant_bytes);
int32_t leer_memoria(Vmx *vmx, uint16_t dir_fisica, uint8_t cant_bytes);
void escribir_memoria(Vmx *vmx, uint16_t dir_fisica, uint8_t cant_bytes, int32_t valor);

#endif // MEMORIA_H
