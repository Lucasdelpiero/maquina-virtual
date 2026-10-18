#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include "vmx.h"

// Retorna el mnemonico de la instruccion dado su codigo de operacion (0..31)
const char* obtener_mnemonico(int opc);

// Retorna el nombre del registro dado su codigo numerico (0..31)
const char* obtener_nombre_registro(int cod_reg);

// Formatea un operando en la cadena destino segun su tipo y dato crudo
void formatear_operando(char destino[], int tipo, int32_t dato);

// Desensambla la instruccion ubicada en la direccion fisica especificada
// Retorna la cantidad de bytes consumidos por la instruccion
int desensamblar_en_direccion(Vmx *vmx, int dir_fisica, char buffer[], int tam_buffer);

// Recorre secuencialmente el segmento de codigo e imprime el desensamblado completo (1 vez por instruccion)
void desensamblar_programa(Vmx *vmx);

#endif // DISASSEMBLER_H
