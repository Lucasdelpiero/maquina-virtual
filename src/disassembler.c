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
        cod_reg = dato & 0x1F;
        sprintf(destino, "%s", obtener_nombre_registro(cod_reg));
    } else if (tipo == TIPO_INMEDIATO) {
        valor_inmediato = extender_signo_16_a_32(dato & 0xFFFF);
        sprintf(destino, "%d", valor_inmediato);
    } else if (tipo == TIPO_MEMORIA) {
        cod_reg = dato & 0x1F;
        offset = extender_signo_16_a_32((dato >> 8) & 0xFFFF);
        if (offset == 0) {
            sprintf(destino, "[%s]", obtener_nombre_registro(cod_reg));
        } else if (offset > 0) {
            sprintf(destino, "[%s+%d]", obtener_nombre_registro(cod_reg), offset);
        } else {
            sprintf(destino, "[%s%d]", obtener_nombre_registro(cod_reg), offset);
        }
    }
}

int desensamblar_en_direccion(Vmx *vmx, int dir_fisica, char buffer[], int tam_buffer) {
    if (dir_fisica < 0 || dir_fisica >= TAM_MEMORIA_PRINCIPAL) {
        if (tam_buffer > 0) buffer[0] = '\0';
        return 0;
    }

    uint8_t primer_byte = vmx->memoria.mem_principal[dir_fisica];
    int pos = dir_fisica + 1;
    int opc = 0;
    int tipo_a = TIPO_NINGUNO;
    int tipo_b = TIPO_NINGUNO;
    int32_t dato_a = 0;
    int32_t dato_b = 0;

    if (primer_byte == OP_STOP) {
        opc = OP_STOP;
    } else if (((primer_byte >> 4) & 0x03) == 0) {
        opc = primer_byte & 0x1F;
        tipo_a = (primer_byte >> 6) & 0x03;

        if (tipo_a == TIPO_REGISTRO) {
            dato_a = vmx->memoria.mem_principal[pos] & 0x1F;
            pos += 1;
        } else if (tipo_a == TIPO_INMEDIATO) {
            uint16_t val16 = ((uint16_t)vmx->memoria.mem_principal[pos] << 8) | (uint16_t)vmx->memoria.mem_principal[pos + 1];
            dato_a = val16;
            pos += 2;
        } else if (tipo_a == TIPO_MEMORIA) {
            uint16_t offset = ((uint16_t)vmx->memoria.mem_principal[pos] << 8) | (uint16_t)vmx->memoria.mem_principal[pos + 1];
            uint8_t cod_reg = vmx->memoria.mem_principal[pos + 2] & 0x1F;
            dato_a = ((int32_t)offset << 8) | cod_reg;
            pos += 3;
        }
    } else {
        opc = 0x10 | (primer_byte & 0x0F);
        tipo_b = (primer_byte >> 6) & 0x03;
        tipo_a = (primer_byte >> 4) & 0x03;

        // B se codifica primero en el binario
        if (tipo_b == TIPO_REGISTRO) {
            dato_b = vmx->memoria.mem_principal[pos] & 0x1F;
            pos += 1;
        } else if (tipo_b == TIPO_INMEDIATO) {
            uint16_t val16 = ((uint16_t)vmx->memoria.mem_principal[pos] << 8) | (uint16_t)vmx->memoria.mem_principal[pos + 1];
            dato_b = val16;
            pos += 2;
        } else if (tipo_b == TIPO_MEMORIA) {
            uint16_t offset = ((uint16_t)vmx->memoria.mem_principal[pos] << 8) | (uint16_t)vmx->memoria.mem_principal[pos + 1];
            uint8_t cod_reg = vmx->memoria.mem_principal[pos + 2] & 0x1F;
            dato_b = ((int32_t)offset << 8) | cod_reg;
            pos += 3;
        }

        // A se codifica segundo
        if (tipo_a == TIPO_REGISTRO) {
            dato_a = vmx->memoria.mem_principal[pos] & 0x1F;
            pos += 1;
        } else if (tipo_a == TIPO_INMEDIATO) {
            uint16_t val16 = ((uint16_t)vmx->memoria.mem_principal[pos] << 8) | (uint16_t)vmx->memoria.mem_principal[pos + 1];
            dato_a = val16;
            pos += 2;
        } else if (tipo_a == TIPO_MEMORIA) {
            uint16_t offset = ((uint16_t)vmx->memoria.mem_principal[pos] << 8) | (uint16_t)vmx->memoria.mem_principal[pos + 1];
            uint8_t cod_reg = vmx->memoria.mem_principal[pos + 2] & 0x1F;
            dato_a = ((int32_t)offset << 8) | cod_reg;
            pos += 3;
        }
    }

    int cant_bytes = pos - dir_fisica;
    const char *mnem = obtener_mnemonico(opc);
    char op_a_str[64];
    char op_b_str[64];
    formatear_operando(op_a_str, tipo_a, dato_a);
    formatear_operando(op_b_str, tipo_b, dato_b);

    char bytes_hex[32] = "";
    char byte_str[8];
    int i;
    for (i = 0; i < cant_bytes && (dir_fisica + i) < TAM_MEMORIA_PRINCIPAL; i++) {
        if (i > 0) {
            strcat(bytes_hex, " "); //dejar espacio entre palabras
        }
        sprintf(byte_str, "%02X", vmx->memoria.mem_principal[dir_fisica + i]);
        strcat(bytes_hex, byte_str);
    }

    if (opc == OP_STOP) {
        snprintf(buffer, tam_buffer, "[%04X] %-17s | %s", dir_fisica, bytes_hex, mnem);
    } else if (opc <= 0x0A) {
        snprintf(buffer, tam_buffer, "[%04X] %-17s | %s %s", dir_fisica, bytes_hex, mnem, op_a_str);
    } else {
        snprintf(buffer, tam_buffer, "[%04X] %-17s | %s %s, %s", dir_fisica, bytes_hex, mnem, op_a_str, op_b_str);
    }

    return cant_bytes;
}

void desensamblar_programa(Vmx *vmx) {
    int dir = 0;
    int tam_codigo = vmx->memoria.tabla_segmentos[0] & 0xFFFF;
    char buffer[128];

    while (dir < tam_codigo) {
        int cant_bytes = desensamblar_en_direccion(vmx, dir, buffer, sizeof(buffer));
        if (cant_bytes <= 0) break;
        printf("%s\n", buffer);
        dir += cant_bytes;
    }
}
