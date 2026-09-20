#ifndef OPERADORES_H
#define OPERADORES_H

#include <stdint.h>
#include "vmx.h"

// Actualiza el registro de condicion (CC: N, Z, C, V) segun el resultado de la operacion
void actualizar_cc(Vmx *vmx, int32_t resultado, int carry, int overflow);

// Inicializa y registra todas las operaciones en la tabla de operaciones tabla_operaciones[32]
void inicializar_operadores(void);

// Declaracion de las 26 operaciones con firma uniforme void (*)(Vmx*)
void op_sys(Vmx *vmx);
void op_jmp(Vmx *vmx);
void op_jp(Vmx *vmx);
void op_jn(Vmx *vmx);
void op_jz(Vmx *vmx);
void op_jc(Vmx *vmx);
void op_jv(Vmx *vmx);
void op_jnp(Vmx *vmx);
void op_jnn(Vmx *vmx);
void op_jnz(Vmx *vmx);
void op_not(Vmx *vmx);
void op_stop(Vmx *vmx);

void op_mov(Vmx *vmx);
void op_add(Vmx *vmx);
void op_sub(Vmx *vmx);
void op_mul(Vmx *vmx);
void op_div(Vmx *vmx);
void op_cmp(Vmx *vmx);
void op_and(Vmx *vmx);
void op_or(Vmx *vmx);
void op_xor(Vmx *vmx);
void op_swap(Vmx *vmx);
void op_shl(Vmx *vmx);
void op_shr(Vmx *vmx);
void op_sar(Vmx *vmx);
void op_ldl(Vmx *vmx);
void op_ldh(Vmx *vmx);
void op_rnd(Vmx *vmx);

#endif // OPERADORES_H
