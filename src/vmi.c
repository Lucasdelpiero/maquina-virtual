#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "vmi.h"
#include "logger.h"
#include "memoria.h"

// Variable estatica interna para recordar la ruta del archivo de imagen
static char ruta_vmi_actual[260] = "";

void vmi_configurar_archivo(char ruta[]) {
    if (ruta != NULL) {
        strcpy(ruta_vmi_actual, ruta);
    } else {
        ruta_vmi_actual[0] = '\0';
    }
}

int vmi_tiene_archivo(void) {
    return (ruta_vmi_actual[0] != '\0');
}

char *vmi_obtener_archivo(void) {
    return ruta_vmi_actual;
}

// Escribe 2 bytes en formato big-endian
static void escribir_uint16_be(FILE *f, int valor) {
    uint8_t buffer[2];
    buffer[0] = (uint8_t)((valor >> 8) & 0xFF);
    buffer[1] = (uint8_t)(valor & 0xFF);
    fwrite(buffer, sizeof(uint8_t), 2, f);
}

// Lee 2 bytes en formato big-endian
static int leer_uint16_be(FILE *f, int *resultado) {
    uint8_t buffer[2];
    if (fread(buffer, sizeof(uint8_t), 2, f) != 2) {
        return 0;
    }
    *resultado = ((int)buffer[0] << 8) | (int)buffer[1];
    return 1;
}

// Escribe 4 bytes en formato big-endian
static void escribir_uint32_be(FILE *f, uint32_t valor) {
    uint8_t buffer[4];
    buffer[0] = (uint8_t)((valor >> 24) & 0xFF);
    buffer[1] = (uint8_t)((valor >> 16) & 0xFF);
    buffer[2] = (uint8_t)((valor >> 8) & 0xFF);
    buffer[3] = (uint8_t)(valor & 0xFF);
    fwrite(buffer, sizeof(uint8_t), 4, f);
}

// Lee 4 bytes en formato big-endian
static int leer_uint32_be(FILE *f, uint32_t *resultado) {
    uint8_t buffer[4];
    if (fread(buffer, sizeof(uint8_t), 4, f) != 4) {
        return 0;
    }
    *resultado = ((uint32_t)buffer[0] << 24) |
                 ((uint32_t)buffer[1] << 16) |
                 ((uint32_t)buffer[2] << 8)  |
                 (uint32_t)buffer[3];
    return 1;
}

int guardar_imagen_vmi(Vmx *vmx, char ruta_archivo[], int tam_mem_kib) {
    FILE *f;
    uint8_t version;
    int i;
    int bytes_memoria;
    char *ruta_final;

    if (vmx == NULL) {
        logger("[ERROR][VMI] Puntero VMX nulo al guardar imagen.\n");
        return 0;
    }

    // Si se pasa ruta valida la usamos, de lo contrario usamos la configurada en el modulo
    if (ruta_archivo != NULL && strlen(ruta_archivo) > 0) {
        ruta_final = ruta_archivo;
    } else if (strlen(ruta_vmi_actual) > 0) {
        ruta_final = ruta_vmi_actual;
    } else {
        logger("[ERROR][VMI] No hay archivo .vmi configurado para guardar.\n");
        return 0;
    }

    // Si el tamano recibido es menor o igual a 0, usamos el de la memoria
    if (tam_mem_kib <= 0) {
        tam_mem_kib = vmx->memoria.tam_mem_kib;
    }

    f = fopen(ruta_final, "wb");
    if (f == NULL) {
        logger("[ERROR][VMI] No se pudo crear o abrir el archivo: '%s'\n", ruta_final);
        return 0;
    }

    // 1. Cabecera (8 bytes)
    // Identificador "VMI26" (5 bytes)
    fwrite("VMI26", sizeof(char), 5, f);

    // Version 1 (1 byte)
    version = 1;
    fwrite(&version, sizeof(uint8_t), 1, f);

    // Tamano de memoria en KiB (2 bytes big-endian)
    escribir_uint16_be(f, tam_mem_kib);

    // 2. Registros (32 x 4 = 128 bytes)
    for (i = 0; i < 32; i++) {
        escribir_uint32_be(f, vmx->registros[i]);
    }

    // 3. Tabla de descriptores de segmentos (8 x 4 = 32 bytes)
    for (i = 0; i < TAM_SEGMENTOS; i++) {
        escribir_uint32_be(f, vmx->memoria.tabla_segmentos[i]);
    }

    // 4. Memoria principal (tam_mem_kib * 1024 bytes)
    bytes_memoria = tam_mem_kib * 1024;
    fwrite(vmx->memoria.mem_principal, sizeof(uint8_t), bytes_memoria, f);

    fclose(f);

    logger("[VMI] Imagen guardada exitosamente en '%s' (%d KiB de memoria).\n", ruta_final, tam_mem_kib);
    return 1;
}

int cargar_imagen_vmi(Vmx *vmx, char ruta_archivo[], int *tam_mem_kib) {
    FILE *f;
    char identificador[6];
    uint8_t version;
    int mem_kib;
    int bytes_memoria;
    int i;
    uint32_t valor32;

    if (vmx == NULL || ruta_archivo == NULL || strlen(ruta_archivo) == 0) {
        logger("[ERROR][VMI] Parametros invalidos para cargar imagen.\n");
        return 0;
    }

    f = fopen(ruta_archivo, "rb");
    if (f == NULL) {
        logger("[ERROR][VMI] No se pudo abrir el archivo de imagen: '%s'\n", ruta_archivo);
        return 0;
    }

    // 1. Leer y validar identificador (5 bytes)
    if (fread(identificador, sizeof(char), 5, f) != 5) {
        fclose(f);
        logger("[ERROR][VMI] Fallo al leer identificador de cabecera en '%s'\n", ruta_archivo);
        return 0;
    }
    identificador[5] = '\0';

    if (strcmp(identificador, "VMI26") != 0) {
        fclose(f);
        logger("[ERROR][VMI] Identificador invalido: '%s' (se esperaba 'VMI26') en '%s'\n", identificador, ruta_archivo);
        return 0;
    }

    // 2. Leer y validar version (1 byte)
    if (fread(&version, sizeof(uint8_t), 1, f) != 1) {
        fclose(f);
        logger("[ERROR][VMI] Fallo al leer version en '%s'\n", ruta_archivo);
        return 0;
    }

    if (version != 1) {
        fclose(f);
        logger("[ERROR][VMI] Version no compatible: %d (se esperaba version 1) en '%s'\n", version, ruta_archivo);
        return 0;
    }

    // 3. Leer tamano de memoria en KiB (2 bytes big-endian)
    if (!leer_uint16_be(f, &mem_kib) || mem_kib <= 0) {
        fclose(f);
        logger("[ERROR][VMI] Tamano de memoria invalido en '%s'\n", ruta_archivo);
        return 0;
    }

    if (tam_mem_kib != NULL) {
        *tam_mem_kib = mem_kib;
    }
    vmx->memoria.tam_mem_kib = mem_kib;


    // 4. Leer registros (32 registros x 4 bytes cada uno)
    for (i = 0; i < 32; i++) {
        if (!leer_uint32_be(f, &valor32)) {
            fclose(f);
            logger("[ERROR][VMI] Archivo truncado al leer registro %d en '%s'\n", i, ruta_archivo);
            return 0;
        }
        vmx->registros[i] = valor32;
    }

    // 5. Leer tabla de descriptores de segmentos (8 entradas x 4 bytes)
    for (i = 0; i < TAM_SEGMENTOS; i++) {
        if (!leer_uint32_be(f, &valor32)) {
            fclose(f);
            logger("[ERROR][VMI] Archivo truncado al leer descriptor de segmento %d en '%s'\n", i, ruta_archivo);
            return 0;
        }
        vmx->memoria.tabla_segmentos[i] = valor32;
    }

    // 6. Leer memoria principal (mem_kib * 1024 bytes)
    bytes_memoria = mem_kib * 1024;
    if ((int)fread(vmx->memoria.mem_principal, sizeof(uint8_t), bytes_memoria, f) != bytes_memoria) {
        fclose(f);
        logger("[ERROR][VMI] Archivo truncado al leer la memoria principal en '%s'\n", ruta_archivo);
        return 0;
    }

    fclose(f);

    // Configuramos internamente la ruta cargada
    vmi_configurar_archivo(ruta_archivo);

    logger("[VMI] Imagen cargada con exito desde '%s' (%d KiB de memoria).\n", ruta_archivo, mem_kib);
    logger("[VMI] Registro IP restaurado: 0x%08X\n", vmx->registros[IP]);
    return 1;
}
