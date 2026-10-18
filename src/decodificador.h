#ifndef DECODIFICADOR_H
#define DECODIFICADOR_H

#include "vmx.h"

// Lee la instruccion apuntada por IP, extrae opcode y operandos, y avanza IP
void decodificar_instruccion(Vmx *vmx);

#endif // DECODIFICADOR_H
