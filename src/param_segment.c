#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "param_segment.h"
#include "logger.h"
#include "memoria.h"

InfoParamSegment construir_param_segment(Vmx *vmx, char nombre_vmx[], char params[][MAX_PARAM_LEN], int nro_params) {
    InfoParamSegment info;
    int offsets[MAX_PARAMS + 1];
    int offset_actual = 0;
    int argc;
    int i;
    int j;
    int len;
    int offset_argv;
    int tamano_total;
    uint32_t ptr_argv_elem;

    // Caso 1: No hay parametros pasados mediante -p
    if (nro_params <= 0) {
        info.existe = 0;
        info.tamano = 0;
        info.argc = 0;
        info.dir_argv = 0xFFFFFFFF; // Puntero -1 a argv segun especificacion de subrutina principal
        if (vmx != NULL) {
            vmx->registros[PS] = -1; // 0xFFFFFFFF segun especificacion si el segmento no existe
        }

        return info;
    }

    // Caso 2: Param Segment definido con -p
    info.existe = 1;
    argc = 1 + nro_params; // param0 (nombre del .vmx) + param1 ... paramN
    info.argc = argc;

    // 1. Calcular offsets de cada string
    // param0: nombre del archivo ejecutable .vmx
    offsets[0] = offset_actual;
    len = strlen(nombre_vmx);
    offset_actual = offset_actual + len + 1; // +1 por el '\0'

    // param1 .. paramN
    for (i = 0; i < nro_params; i++) {
        offsets[i + 1] = offset_actual;
        len = strlen(params[i]);
        offset_actual = offset_actual + len + 1;
    }

    // 2. El arreglo argv comienza inmediatamente a continuacion del ultimo '\0'
    offset_argv = offset_actual;
    tamano_total = offset_argv + (argc * 4);
    info.tamano = tamano_total;

    // Direccion logica a argv: segmento 0 (16 bits MSB = 0), offset_argv (16 bits LSB)
    info.dir_argv = offset_argv & 0xFFFF;

    if (vmx != NULL) {
        int limite_memoria = vmx->memoria.tam_mem_kib * 1024;
        if (tamano_total > limite_memoria) {
            logger("[ERROR][PARAM] Param Segment excede la memoria principal (%d bytes).\n", tamano_total);
            if (vmx->abortar != NULL) {
                vmx->abortar("ERROR: Memoria insuficiente para alojar Param Segment.");
            }
            return info;
        }

        // 3. Escribir strings en memoria fisica a partir de la direccion 0x00000000
        // param0
        len = strlen(nombre_vmx);
        for (j = 0; j < len; j++) {
            vmx->memoria.mem_principal[offsets[0] + j] = (uint8_t)nombre_vmx[j];
        }
        vmx->memoria.mem_principal[offsets[0] + len] = 0x00;

        // param1 .. paramN
        for (i = 0; i < nro_params; i++) {
            len = strlen(params[i]);
            for (j = 0; j < len; j++) {
                vmx->memoria.mem_principal[offsets[i + 1] + j] = (uint8_t)params[i][j];
            }
            vmx->memoria.mem_principal[offsets[i + 1] + len] = 0x00;
        }

        // 4. Escribir arreglo argv con los argc punteros de 4 bytes en big-endian
        for (i = 0; i < argc; i++) {
            int pos_puntero = offset_argv + (i * 4);
            ptr_argv_elem = offsets[i] & 0xFFFF; // Entrada 0x0000 en 16 bits altos

            vmx->memoria.mem_principal[pos_puntero]     = (uint8_t)((ptr_argv_elem >> 24) & 0xFF);
            vmx->memoria.mem_principal[pos_puntero + 1] = (uint8_t)((ptr_argv_elem >> 16) & 0xFF);
            vmx->memoria.mem_principal[pos_puntero + 2] = (uint8_t)((ptr_argv_elem >> 8) & 0xFF);
            vmx->memoria.mem_principal[pos_puntero + 3] = (uint8_t)(ptr_argv_elem & 0xFF);
        }

        // 5. Configura registro PS y la posicion 0 de la tabla de descriptores de segmentos
        // Param Segment siempre se ubica a partir de 0x00000000 (Base=0, Tamano=tamano_total)
        vmx->registros[PS] = 0x00000000;
        vmx->memoria.tabla_segmentos[0] = tamano_total & 0xFFFF;

        logger("[PARAM] Param Segment creado exitosamente.\n");
        logger("[PARAM]   Tamano total: %d bytes (Strings: %d bytes, Arreglo argv: %d bytes)\n",
               tamano_total, offset_argv, argc * 4);
        logger("[PARAM]   argc: %d, dir_argv: 0x%08X\n",
               info.argc, info.dir_argv);
        logger("[PARAM]   Entrada 0 configurada: Base=0, Tamano=%d | Registro PS=0x%08X\n",
               tamano_total, vmx->registros[PS]);
    }

    return info;
}
