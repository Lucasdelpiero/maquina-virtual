#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "memoria.h"
#include "errores.h"
#include "logger.h"

/*
    Tareas del modulo memoria:
        -Valida accesos a memoria (Dentro de segmento correcto y dentro del limite de memoria)
        -Modifica valores de registros LAR, MAR, MBR cuando se usan operandos de memoria
        -Gestiona acceso a memoria principal:
            -Para esto traduce posicion de memoria logica a fisica usando tabla de segmentos
            -Tambien usa la cant de bytes accedidos, mas que nada para el decodificador y el operador MOV
*/

int32_t get_dir_fisica(Memoria mem, int32_t dir_logica) {
    int16_t segmento = (dir_logica >> 16) & 0xFFFF;
    int16_t offset = dir_logica & 0xFFFF;

    if (segmento < 0 || segmento >= TAM_SEGMENTOS || mem.tabla_segmentos[segmento] == 0xFFFFFFFF) {
        logger("[ERROR][MEMORIA] Segmento invalido o no asignado: segmento=%d, dir_logica=0x%08X\n", segmento, dir_logica);
        abortar("ERROR: Segmento invalido o no asignado.");
    }

    int32_t dir_base = (mem.tabla_segmentos[segmento] >> 16) & 0xFFFF;
    int32_t dir_fisica = dir_base + offset;

    return dir_fisica;
}

int32_t get_valor_memoria(Memoria mem, int32_t dir_logica, int bytes_accedidos) {
    int32_t resultado = 0;
    int i = 0;
    int16_t segmento = (dir_logica >> 16) & 0xFFFF;
    int16_t offset = dir_logica & 0xFFFF;

    if (segmento < 0 || segmento >= TAM_SEGMENTOS || mem.tabla_segmentos[segmento] == 0xFFFFFFFF) {
        logger("[ERROR][MEMORIA] Segmento invalido o no asignado: segmento=%d, dir_logica=0x%08X\n", segmento, dir_logica);
        abortar("ERROR: Segmento invalido o no asignado.");
    }

    int32_t dir_base = (mem.tabla_segmentos[segmento] >> 16) & 0xFFFF;
    int32_t tam_segmento = (mem.tabla_segmentos[segmento] & 0xFFFF);
    int32_t dir_fisica = dir_base + offset;

    int32_t limite_segmento = dir_base + tam_segmento;
    int32_t limite_acceso = dir_fisica + bytes_accedidos;

    if (dir_base > dir_fisica) {
        logger("[ERROR][MEMORIA] Direccion base (%d) mayor que fisica (%d) en segmento %d\n", dir_base, dir_fisica, segmento);
        abortar("ERROR: Direccion base mayor que direccion fisica.");
    }
    if (limite_segmento < limite_acceso) {
        logger("[ERROR][MEMORIA] Limite de segmento (%d) menor que limite de acceso (%d) en segmento %d\n", limite_segmento, limite_acceso, segmento);
        abortar("ERROR: Limite de segmento menor que limite de acceso.");
    }

    // Concatena el valor segun la cantidad de bytes requeridos
    resultado = mem.mem_principal[dir_fisica];
    for (i = 1; i < bytes_accedidos; i++) {
        resultado = resultado << 8;
        resultado += mem.mem_principal[dir_fisica + i];
    }

    return resultado;
}

// Escribe byte a byte
void escribir_byte(Memoria *mem, int pos, uint8_t dato) {
    // Validar memoria
    if (pos >= TAM_MEMORIA_PRINCIPAL) {
        logger("[ERROR][MEMORIA] Desborde de memoria principal: pos=%d (limite %d)\n", pos, TAM_MEMORIA_PRINCIPAL);
        abortar("ERROR: Memoria principal se lleno.");
    } else if (pos < 0) {
        logger("[ERROR][MEMORIA] Acceso a posicion negativa en memoria: pos=%d\n", pos);
        abortar("ERROR: Se quiere acceder a posicion negativa.");
    }

    // Agrega dato leido a la memoria principal
    mem->mem_principal[pos] = dato;
}

int32_t set_valor_memoria(Memoria *mem, int32_t dir_logica, int32_t dato, int cant_accedidos) {
    int32_t dir_fisica;
    int i;
    int bytes[4];
    int offset = 0;
    int primer_byte;

    if (cant_accedidos < 1 || cant_accedidos > 4) {
        logger("[ERROR][MEMORIA] Cantidad de bytes a acceder invalida: %d (debe ser 1..4)\n", cant_accedidos);
        abortar("ERROR: Cantidad de bytes a acceder invalida.");
    }

    dir_fisica = get_dir_fisica(*mem, dir_logica);

    // Separa un entero de 32 bits en 4 elementos de 8 bits big-endian
    for (i = 0; i < 4; i++) {
        bytes[3 - i] = (dato >> (8 * i)) & 0xFF;
    }

    // Escribe byte a byte en memoria
    primer_byte = 4 - cant_accedidos;
    for (i = primer_byte; i < 4; i++) {
        escribir_byte(mem, dir_fisica + offset, (uint8_t)bytes[i]);
        offset++;
    }

    return 0;
}

void inicializar_memoria(Memoria *mem) {
    int i;
    for (i = 0; i < TAM_MEMORIA_PRINCIPAL; i++) {
        mem->mem_principal[i] = 0;
    }
    for (i = 0; i < TAM_SEGMENTOS; i++) {
        mem->tabla_segmentos[i] = 0xFFFFFFFF;
    }
}

// ============================================================================
// Interfaz de memoria de la maquina virtual
// ============================================================================

uint16_t traducir_direccion(Memoria *mem, int32_t dir_logica, uint16_t cant_bytes) {
    int segmento = (dir_logica >> 16) & 0xFFFF;
    int offset = dir_logica & 0xFFFF;
    int dir_base;
    int tam_segmento;
    int dir_fisica;

    // Valida que el codigo de segmento este dentro del rango y no sea invalido (-1)
    if (segmento < 0 || segmento >= TAM_SEGMENTOS || mem->tabla_segmentos[segmento] == 0xFFFFFFFF) {
        logger("[ERROR][MEMORIA] Fallo de segmento: codigo=%d fuera de rango [0..7] o no asignado (0xFFFFFFFF). DirLogica=0x%08X\n",
               segmento, dir_logica);
        abortar("Fallo de segmento: codigo de segmento invalido o no asignado.");
    }

    dir_base = (int)((mem->tabla_segmentos[segmento] >> 16) & 0xFFFF);
    tam_segmento = (int)(mem->tabla_segmentos[segmento] & 0xFFFF);

    // Valida que el acceso no sobrepase el tamano del segmento
    if (offset < 0 || (offset + cant_bytes) > tam_segmento) {
        logger("[ERROR][MEMORIA] Fallo de segmento: offset=%d + cant_bytes=%d excede tamano=%d en segmento %d (base=%d). DirLogica=0x%08X\n",
               offset, cant_bytes, tam_segmento, segmento, dir_base, dir_logica);
        abortar("Fallo de segmento: acceso fuera de los limites del segmento.");
    }

    dir_fisica = dir_base + offset;

    // Valida que la direccion fisica no sobrepase el limite de la memoria principal
    if (dir_fisica + cant_bytes > TAM_MEMORIA_PRINCIPAL) {
        logger("[ERROR][MEMORIA] Fallo de segmento: dir_fisica=%d + cant_bytes=%d excede memoria principal (16384). DirLogica=0x%08X\n",
               dir_fisica, cant_bytes, dir_logica);
        abortar("Fallo de segmento: acceso fuera de la memoria principal.");
    }

    logger("[MEMORIA] Traducir: DirLogica=0x%08X (seg=%d, off=%d) -> DirFisica=0x%04X\n",
           dir_logica, segmento, offset, dir_fisica);

    return (uint16_t)dir_fisica;
}

int32_t leer_memoria(Memoria *mem, uint16_t dir_fisica, uint8_t cant_bytes) {
    int32_t resultado = 0;
    int i;

    if (cant_bytes < 1 || cant_bytes > 4 || (int)dir_fisica + cant_bytes > TAM_MEMORIA_PRINCIPAL) {
        logger("[ERROR][MEMORIA] Lectura invalida: dir_fisica=0x%04X, cant_bytes=%d (limite 16384)\n",
               dir_fisica, cant_bytes);
        abortar("Fallo de segmento: intento de leer fuera de la memoria fisica.");
    }

    resultado = (int32_t)mem->mem_principal[dir_fisica];
    for (i = 1; i < cant_bytes; i++) {
        resultado = (resultado << 8) | (int32_t)mem->mem_principal[dir_fisica + i];
    }

    logger("[MEMORIA] Leer %d byte(s) en DirFisica=0x%04X => 0x%08X (%d)\n",
           cant_bytes, dir_fisica, resultado, resultado);

    return resultado;
}

void escribir_memoria(Memoria *mem, uint16_t dir_fisica, uint8_t cant_bytes, int32_t valor) {
    int bytes[4];
    int i;
    int offset = 0;
    int primer_byte;

    if (cant_bytes < 1 || cant_bytes > 4 || (int)dir_fisica + cant_bytes > TAM_MEMORIA_PRINCIPAL) {
        logger("[ERROR][MEMORIA] Escritura invalida: dir_fisica=0x%04X, cant_bytes=%d, valor=0x%08X\n",
               dir_fisica, cant_bytes, valor);
        abortar("Fallo de segmento: intento de escribir fuera de la memoria fisica.");
    }

    // Descompone el valor de 32 bits en bytes big-endian
    for (i = 0; i < 4; i++) {
        bytes[3 - i] = (valor >> (8 * i)) & 0xFF;
    }

    primer_byte = 4 - cant_bytes;
    for (i = primer_byte; i < 4; i++) {
        mem->mem_principal[dir_fisica + offset] = (uint8_t)bytes[i];
        offset++;
    }

    logger("[MEMORIA] Escribir %d byte(s) en DirFisica=0x%04X <= valor=0x%08X (%d)\n",
           cant_bytes, dir_fisica, valor, valor);
}
