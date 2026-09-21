#include "decodificador.h"
#include "operandos.h"
#include "logger.h"
#include <stdio.h>
#include <stdint.h>

/*
    Tareas del decodificador:
    - Lee el primer byte de la instruccion apuntada por IP
    - Extrae el codigo de operacion (OPC) y los tipos de operandos
    - Lee los operandos del binario (en orden inverso en el binario: primero B y luego A)
    - Empaqueta en los registros OPC, OP1 (operando A) y OP2 (operando B)
    - Avanza el registro IP sumando el tamano de la instruccion
    - No modifica LAR, MAR ni MBR
*/

// Lee un operando de la memoria fisica del segmento de codigo
static int32_t leer_operando(Vmx *vmx, int tipo, int *pos) {
    int32_t dato = 0;
    int p = *pos;

    if (tipo == TIPO_REGISTRO) {
        // 1 byte: codigo de registro en los 5 bits menos significativos
        dato = vmx->memoria.mem_principal[p] & 0x1F;
        *pos = p + 1;
    } else if (tipo == TIPO_INMEDIATO) {
        // 2 bytes big-endian: valor entero de 16 bits
        uint16_t val16 = ((uint16_t)vmx->memoria.mem_principal[p] << 8) | (uint16_t)vmx->memoria.mem_principal[p + 1];
        dato = val16;
        *pos = p + 2;
    } else if (tipo == TIPO_MEMORIA) {
        // 3 bytes: 2 bytes de offset big-endian + 1 byte con codigo de registro (5 bits bajos)
        uint16_t offset = ((uint16_t)vmx->memoria.mem_principal[p] << 8) | (uint16_t)vmx->memoria.mem_principal[p + 1];
        uint8_t cod_reg = vmx->memoria.mem_principal[p + 2] & 0x1F;
        dato = ((int32_t)offset << 8) | cod_reg;
        *pos = p + 3;
    }

    return dato;
}

void decodificar_instruccion(Vmx *vmx) {
    int offset_ip;
    int tam_codigo;
    uint8_t primer_byte;
    int opc = 0;
    int tipo_a = TIPO_NINGUNO;
    int tipo_b = TIPO_NINGUNO;
    int32_t dato_a = 0;
    int32_t dato_b = 0;
    int pos;
    int tam_instruccion;

    // IP es una direccion logica relativa al segmento de codigo (segmento 0)
    offset_ip = vmx->registros[IP] & 0xFFFF;
    tam_codigo = vmx->memoria.tabla_segmentos[0] & 0xFFFF;

    if (offset_ip < 0 || offset_ip >= tam_codigo) {
        logger("[ERROR][DECODER] Intento de decodificar fuera del segmento de codigo: offset_ip=%d, tam_codigo=%d, IP=0x%08X\n",
               offset_ip, tam_codigo, vmx->registros[IP]);
        vmx->abortar("Error: Intento de decodificar instruccion fuera del segmento de codigo.");
    }

    primer_byte = vmx->memoria.mem_principal[offset_ip];
    pos = offset_ip + 1;

    if (primer_byte == OP_STOP) {
        // Instruccion sin operandos (STOP, 0x0F)
        opc = OP_STOP;
        tipo_a = TIPO_NINGUNO;
        tipo_b = TIPO_NINGUNO;
    } else if (((primer_byte >> 4) & 0x03) == 0) {
        // Instruccion con un solo operando (0x00 a 0x0A: SYS, JMP, Jcc, NOT)
        opc = primer_byte & 0x1F;
        tipo_a = (primer_byte >> 6) & 0x03;
        tipo_b = TIPO_NINGUNO;

        // Lee el operando A
        dato_a = leer_operando(vmx, tipo_a, &pos);
    } else {
        // Instruccion con dos operandos (0x10 a 0x1F: MOV, ADD, SUB, ...)
        opc = 0x10 | (primer_byte & 0x0F);
        tipo_b = (primer_byte >> 6) & 0x03;
        tipo_a = (primer_byte >> 4) & 0x03;

        // En el binario se codifica primero el operando B y luego el A
        dato_b = leer_operando(vmx, tipo_b, &pos);
        dato_a = leer_operando(vmx, tipo_a, &pos);
    }

    tam_instruccion = pos - offset_ip;

    // Empaqueta en registros OPC, OP1 y OP2
    vmx->registros[OPC] = opc;
    vmx->registros[OP1] = (tipo_a << 24) | (dato_a & 0x00FFFFFF);
    vmx->registros[OP2] = (tipo_b << 24) | (dato_b & 0x00FFFFFF);

    // Avanza el registro IP sumando los bytes consumidos por la instruccion
    vmx->registros[IP] += tam_instruccion;

    logger("[DECODER] IP_prev=0x%04X Opcode=0x%02X (%d bytes) OP1=(tipo=%d dato=0x%X) OP2=(tipo=%d dato=0x%X) -> IP_nuevo=0x%08X\n",
           offset_ip, opc, tam_instruccion, tipo_a, dato_a, tipo_b, dato_b, vmx->registros[IP]);
}
