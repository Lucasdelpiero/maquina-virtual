#ifndef DEPURADOR_H
#define DEPURADOR_H

#include "vmx.h"

// Maneja la ejecucion de un breakpoint (SYS 0x0F).
// Si no hay archivo .vmi configurado, se ignora y no realiza accion.
// Si hay archivo .vmi, guarda la imagen actual y espera una accion (g, q, Enter).
void depurador_manejar_breakpoint(Vmx *vmx);

// Verifica si la maquina virtual esta en modo paso a paso.
// Si esta activo, dispara automaticamente un breakpoint tras ejecutar la instruccion.
void depurador_verificar_paso_a_paso(Vmx *vmx);

#endif // DEPURADOR_H
