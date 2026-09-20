#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include "vmx.h"

// Retorna el mnemonico de la instruccion dado su codigo de operacion (0..31)
const char* obtener_mnemonico(int opc);

// Retorna el nombre del registro dado su codigo numerico (0..31)
const char* obtener_nombre_registro(int cod_reg);

// Formatea un operando en la cadena destino segun su tipo y dato crudo
void formatear_operando(char destino[], int tipo, int32_t dato);

// Desensambla la instruccion actual y formatea la linea completa en el buffer provisto
// Formato: [XXXX] XX XX ... | MNEM OP_A, OP_B
void desensamblar_instruccion(Vmx *vmx, char buffer[], int tam_buffer);

#endif // DISASSEMBLER_H
