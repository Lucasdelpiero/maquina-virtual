#ifndef OPERADORES_H
#define OPERADORES_H

#include <stdint.h>
#include "vmx.h"

// Actualiza el registro de condicion (CC: N, Z, C, V) segun el resultado
void actualizar_cc(Vmx *vmx, int32_t resultado, int carry, int overflow);

#endif // OPERADORES_H
