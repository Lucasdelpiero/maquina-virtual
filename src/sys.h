#ifndef SYS_H
#define SYS_H

#include "vmx.h"

// Implementacion de la instruccion SYS (llamadas al sistema READ=1 y WRITE=2)
void op_sys(Vmx *vmx);

#endif // SYS_H
