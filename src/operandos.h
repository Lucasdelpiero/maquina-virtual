#ifndef OPERANDOS_H
#define OPERANDOS_H

#include <stdint.h>
#include "vmx.h"

// Codigos de tipos de operandos segun la especificacion
typedef enum {
    TIPO_NINGUNO   = 0, // 00 binario: 0 bytes
    TIPO_REGISTRO  = 1, // 01 binario: 1 byte
    TIPO_INMEDIATO = 2, // 10 binario: 2 bytes
    TIPO_MEMORIA   = 3  // 11 binario: 3 bytes
} TipoOperando;

// Extrae el codigo de tipo de operando (8 bits superiores)
int get_tipo(int32_t operando);

// Extrae el dato crudo del operando (24 bits inferiores)
int32_t get_dato(int32_t operando);

// Extiende el signo de un valor de 16 bits en complemento a dos hacia 32 bits
int32_t extender_signo_16_a_32(uint16_t valor16);

// Resuelve la direccion fisica combinando el registro base y el desplazamiento con signo
uint16_t resolver_direccion_memoria(Vmx *vmx, int32_t dato);

// Obtiene el valor numerico de 32 bits a partir del tipo y dato de un operando
int32_t get_valor(Vmx *vmx, int tipo, int32_t dato);

// Almacena un valor de 32 bits en el destino (registro o memoria; aborta si es inmediato)
void set_valor(Vmx *vmx, int tipo, int32_t dato, int32_t valor);

// Funcion especial para LDH y LDL: ciclo read-modify-write para modificar solo 16 bits
void combinar_mitad(Vmx *vmx, int tipo, int32_t dato, uint16_t mitad_nueva, int cargar_alta);

#endif // OPERANDOS_H
