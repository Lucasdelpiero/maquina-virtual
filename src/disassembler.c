#include "disassembler.h"
#include "operandos.h"
#include <stdio.h>
#include <string.h>

// Tabla de mnemonicos indexada por codigo de operacion (0..31)
static const char *tabla_mnemonicos[32] = {
    [0x00] = "SYS",
    [0x01] = "JMP",
    [0x02] = "JP",
    [0x03] = "JN",
    [0x04] = "JZ",
    [0x05] = "JC",
    [0x06] = "JV",
    [0x07] = "JNP",
    [0x08] = "JNN",
    [0x09] = "JNZ",
    [0x0A] = "NOT",
    [0x0F] = "STOP",
    [0x10] = "MOV",
    [0x11] = "ADD",
    [0x12] = "SUB",
    [0x13] = "MUL",
    [0x14] = "DIV",
    [0x15] = "CMP",
    [0x16] = "AND",
    [0x17] = "OR",
    [0x18] = "XOR",
    [0x19] = "SWAP",
    [0x1A] = "SHL",
    [0x1B] = "SHR",
    [0x1C] = "SAR",
    [0x1D] = "LDL",
    [0x1E] = "LDH",
    [0x1F] = "RND"
};

// Tabla de nombres de registros indexada por codigo (0..31)
static const char *tabla_registros[32] = {
    [0]  = "IP",
    [1]  = "OPC",
    [2]  = "OP1",
    [3]  = "OP2",
    [4]  = "LAR",
    [5]  = "MAR",
    [6]  = "MBR",
    [10] = "EAX",
    [11] = "EBX",
    [12] = "ECX",
    [13] = "EDX",
    [14] = "EEX",
    [15] = "EFX",
    [16] = "AC",
    [17] = "CC",
    [26] = "CS",
    [27] = "DS"
};

const char* obtener_mnemonico(int opc) {
    if (opc >= 0 && opc < 32 && tabla_mnemonicos[opc] != NULL) {
        return tabla_mnemonicos[opc];
    }
    return "???";
}

const char* obtener_nombre_registro(int cod_reg) {
    if (cod_reg >= 0 && cod_reg < 32 && tabla_registros[cod_reg] != NULL) {
        return tabla_registros[cod_reg];
    }
    return "???";
}

void formatear_operando(char destino[], int tipo, int32_t dato) {
    int cod_reg;
    int32_t offset;
    int32_t valor_inmediato;

    destino[0] = '\0';

    if (tipo == TIPO_REGISTRO) {
        cod_reg = (int)(dato & 0x1F);
        sprintf(destino, "%s", obtener_nombre_registro(cod_reg));
    } else if (tipo == TIPO_INMEDIATO) {
        valor_inmediato = extender_signo_16_a_32((uint16_t)(dato & 0xFFFF));
        sprintf(destino, "%d", valor_inmediato);
    } else if (tipo == TIPO_MEMORIA) {
        cod_reg = (int)(dato & 0x1F);
        offset = extender_signo_16_a_32((uint16_t)((dato >> 8) & 0xFFFF));
        if (offset == 0) {
            sprintf(destino, "[%s]", obtener_nombre_registro(cod_reg));
        } else if (offset > 0) {
            sprintf(destino, "[%s+%d]", obtener_nombre_registro(cod_reg), offset);
        } else {
            sprintf(destino, "[%s%d]", obtener_nombre_registro(cod_reg), offset);
        }
    }
}

void desensamblar_instruccion(Vmx *vmx, char buffer[], int tam_buffer) {
    int opc;
    int tipo_a;
    int32_t dato_a;
    int tipo_b;
    int32_t dato_b;
    char op_a_str[64];
    char op_b_str[64];
    const char *mnem;
    int dir_instruccion;

    opc = (int)vmx->registros[OPC];
    tipo_a = get_tipo(vmx->registros[OP1]);
    dato_a = get_dato(vmx->registros[OP1]);
    tipo_b = get_tipo(vmx->registros[OP2]);
    dato_b = get_dato(vmx->registros[OP2]);

    mnem = obtener_mnemonico(opc);

    formatear_operando(op_a_str, tipo_a, dato_a);
    formatear_operando(op_b_str, tipo_b, dato_b);

    // Calcula el tamano de la instruccion segun sus operandos
    int cant_bytes = 1;
    if (opc != OP_STOP) {
        if (opc <= 0x0A) {
            cant_bytes += (tipo_a == TIPO_REGISTRO ? 1 : (tipo_a == TIPO_INMEDIATO ? 2 : (tipo_a == TIPO_MEMORIA ? 3 : 0)));
        } else {
            cant_bytes += (tipo_a == TIPO_REGISTRO ? 1 : (tipo_a == TIPO_INMEDIATO ? 2 : (tipo_a == TIPO_MEMORIA ? 3 : 0)));
            cant_bytes += (tipo_b == TIPO_REGISTRO ? 1 : (tipo_b == TIPO_INMEDIATO ? 2 : (tipo_b == TIPO_MEMORIA ? 3 : 0)));
        }
    }

    // Si IP ya fue avanzado por el decodificador, la instruccion comenzo en IP - cant_bytes
    int ip_actual = (int)(vmx->registros[IP] & 0xFFFF);
    if (ip_actual >= cant_bytes) {
        dir_instruccion = ip_actual - cant_bytes;
    } else {
        dir_instruccion = ip_actual;
    }

    // Formatea los bytes en hexadecimal
    char bytes_hex[32] = "";
    char byte_str[8];
    int i;
    for (i = 0; i < cant_bytes && (dir_instruccion + i) < 16384; i++) {
        sprintf(byte_str, "%02X ", vmx->memoria.mem_principal[dir_instruccion + i]);
        strcat(bytes_hex, byte_str);
    }

    if (opc == OP_STOP) {
        snprintf(buffer, tam_buffer, "[%04X] %-18s | %s", dir_instruccion, bytes_hex, mnem);
    } else if (opc <= 0x0A) {
        snprintf(buffer, tam_buffer, "[%04X] %-18s | %s %s", dir_instruccion, bytes_hex, mnem, op_a_str);
    } else {
        snprintf(buffer, tam_buffer, "[%04X] %-18s | %s %s, %s", dir_instruccion, bytes_hex, mnem, op_a_str, op_b_str);
    }
}
