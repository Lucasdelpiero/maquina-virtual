#include "inicializador.h"
#include "vmx.h"
#include "memoria.h"
#include "errores.h"
#include "logger.h"
#include "operadores.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/*
    Tareas del inicializador:
    -Lee el archivo en binario
    -Valida la version del .vmx
    -Vuelca datos a la memoria principal y tabla de segmentos
    -Escribe en registros las posiciones de CS, DS e IP
*/

// Toma el valor de tamano de codigo dado y la constante de tamano de MEM PRINCIPAL
// y lo usa para calcular la tabla de segmentos (codigo y datos)
void setear_tabla_segmento(Memoria *mem, int tamCS) {
    int16_t inicio_cs = 0;
    int16_t fin_cs = tamCS;
    int16_t inicio_ds = fin_cs;
    int16_t fin_ds = TAM_MEMORIA_PRINCIPAL - fin_cs;

    mem->tabla_segmentos[0] = ((uint32_t)inicio_cs << 16) | ((uint32_t)fin_cs & 0xFFFF);
    mem->tabla_segmentos[1] = ((uint32_t)inicio_ds << 16) | ((uint32_t)fin_ds & 0xFFFF);

    int i;
    for (i = 2; i < TAM_SEGMENTOS; i++) {
        mem->tabla_segmentos[i] = 0xFFFFFFFF;
    }
}

// Lectura y carga del archivo binario .vmx en memoria
int leer_archivo(char archNom[], Memoria *mem) {
    inicializar_memoria(mem);
    uint8_t byteLeido;
    char identificador[6];
    int version;
    int tamCS = 0;
    int temp[2];
    int i;

    FILE *f = fopen(archNom, "rb");
    if (f == NULL) {
        logger("[ERROR][CARGA] No se pudo abrir el archivo especificado: '%s'\n", archNom);
        abortar("Error: No se pudo abrir el archivo .vmx especificado.");
        return -1;
    }

    // Carga identificador (5 bytes)
    for (i = 0; i < 5; i++) {
        if (fread(&byteLeido, sizeof(uint8_t), 1, f) != 1) {
            fclose(f);
            logger("[ERROR][CARGA] Fallo al leer identificador de cabecera en '%s'\n", archNom);
            abortar("Error: No se pudo leer el identificador de cabecera.");
            return -1;
        }
        identificador[i] = (char)byteLeido;
    }
    identificador[5] = '\0';

    if (strcmp(identificador, "VMX26") != 0) {
        fclose(f);
        logger("[ERROR][CARGA] Identificador invalido: '%s' (se esperaba 'VMX26') en '%s'\n", identificador, archNom);
        abortar("Error: Identificador de cabecera invalido. Se esperaba 'VMX26'.");
        return -1;
    }

    // Carga version (1 byte)
    if (fread(&byteLeido, sizeof(uint8_t), 1, f) != 1) {
        fclose(f);
        logger("[ERROR][CARGA] Fallo al leer version en '%s'\n", archNom);
        abortar("Error: No se pudo leer la version del archivo .vmx.");
        return -1;
    }
    version = byteLeido;
    if (version != 1) {
        fclose(f);
        logger("[ERROR][CARGA] Version %d no compatible (se esperaba 1) en '%s'\n", version, archNom);
        abortar("Error: Version de archivo .vmx no compatible. Se esperaba version 1.");
        return -1;
    }

    // Carga tamano de codigo big-endian (2 bytes)
    for (i = 0; i < 2; i++) {
        if (fread(&byteLeido, sizeof(uint8_t), 1, f) != 1) {
            fclose(f);
            logger("[ERROR][CARGA] Fallo al leer tamano de codigo en '%s'\n", archNom);
            abortar("Error: No se pudo leer el tamano de codigo.");
            return -1;
        }
        temp[i] = byteLeido;
    }
    tamCS = (temp[0] << 8) | temp[1];

    if (tamCS <= 0 || tamCS > TAM_MEMORIA_PRINCIPAL) {
        fclose(f);
        logger("[ERROR][CARGA] Tamano de codigo invalido: %d bytes (limite 1..%d) en '%s'\n", tamCS, TAM_MEMORIA_PRINCIPAL, archNom);
        abortar("Error: Tamano de codigo invalido en la cabecera.");
        return -1;
    }

    // Carga cada byte leido a la memoria principal
    i = 0;
    while (i < tamCS && fread(&byteLeido, sizeof(uint8_t), 1, f) == 1) {
        escribir_byte(mem, i, byteLeido);
        i++;
    }

    fclose(f);
    setear_tabla_segmento(mem, tamCS);
    return 0;
}

void inicializar_vmx(Vmx *vmx, int modo_debug, int modo_disassembler) {
    int i;
    vmx->modo_debug = modo_debug;
    vmx->modo_disassembler = modo_disassembler;
    vmx->abortar = abortar;

    logger_habilitar(modo_debug);
    logger("[INIT] Inicializando VMX: modo_debug=%d, modo_disassembler=%d.\n", modo_debug, modo_disassembler);

    // Inicializa los 32 registros en cero
    for (i = 0; i < 32; i++) {
        vmx->registros[i] = 0;
    }

    // Inicializa la memoria principal y tabla de descriptores
    inicializar_memoria(&vmx->memoria);

    // Inicializa la tabla de operaciones
    inicializar_operadores();
}

void cargar_programa(Vmx *vmx, char ruta_archivo[]) {
    FILE *f;
    uint8_t byteLeido;
    char identificador[6];
    int version;
    int tamCS = 0;
    int temp[2];
    int i;

    f = fopen(ruta_archivo, "rb");
    if (f == NULL) {
        logger("[ERROR][CARGA] No se pudo abrir el archivo: '%s'\n", ruta_archivo);
        vmx->abortar("Error: No se pudo abrir el archivo .vmx especificado.");
    }

    // Carga identificador (5 bytes)
    for (i = 0; i < 5; i++) {
        if (fread(&byteLeido, sizeof(uint8_t), 1, f) != 1) {
            fclose(f);
            logger("[ERROR][CARGA] Fallo al leer identificador de cabecera en '%s'\n", ruta_archivo);
            vmx->abortar("Error: No se pudo leer el identificador de cabecera.");
        }
        identificador[i] = (char)byteLeido;
    }
    identificador[5] = '\0';

    if (strcmp(identificador, "VMX26") != 0) {
        fclose(f);
        logger("[ERROR][CARGA] Identificador invalido: '%s' (se esperaba 'VMX26') en '%s'\n", identificador, ruta_archivo);
        vmx->abortar("Error: Identificador de cabecera invalido. Se esperaba 'VMX26'.");
    }

    // Carga version (1 byte)
    if (fread(&byteLeido, sizeof(uint8_t), 1, f) != 1) {
        fclose(f);
        logger("[ERROR][CARGA] Fallo al leer version en '%s'\n", ruta_archivo);
        vmx->abortar("Error: No se pudo leer la version del archivo .vmx.");
    }
    version = byteLeido;
    if (version != 1) {
        fclose(f);
        logger("[ERROR][CARGA] Version %d no compatible (se esperaba 1) en '%s'\n", version, ruta_archivo);
        vmx->abortar("Error: Version de archivo .vmx no compatible. Se esperaba version 1.");
    }

    // Carga tamano de codigo big-endian (2 bytes)
    for (i = 0; i < 2; i++) {
        if (fread(&byteLeido, sizeof(uint8_t), 1, f) != 1) {
            fclose(f);
            logger("[ERROR][CARGA] Fallo al leer tamano de codigo en '%s'\n", ruta_archivo);
            vmx->abortar("Error: No se pudo leer el tamano de codigo.");
        }
        temp[i] = byteLeido;
    }
    tamCS = (temp[0] << 8) | temp[1];

    if (tamCS <= 0 || tamCS > 16384) {
        fclose(f);
        logger("[ERROR][CARGA] Tamano de codigo invalido: %d bytes (debe ser entre 1 y 16384) en '%s'\n", tamCS, ruta_archivo);
        vmx->abortar("Error: Tamano de codigo invalido en la cabecera del archivo .vmx.");
    }

    // Carga el codigo del programa en memoria fisica a partir de la posicion 0
    i = 0;
    while (i < tamCS && fread(&byteLeido, sizeof(uint8_t), 1, f) == 1) {
        vmx->memoria.mem_principal[i] = byteLeido;
        i++;
    }

    if (i != tamCS) {
        fclose(f);
        logger("[ERROR][CARGA] Archivo truncado: se esperaban %d bytes de codigo y solo se leyeron %d en '%s'\n", tamCS, i, ruta_archivo);
        vmx->abortar("Error: El archivo .vmx termino antes de lo esperado o esta danado.");
    }

    fclose(f);

    // Configura la tabla de descriptores de segmentos:
    // Entrada 0 (Codigo): Base = 0, Tamano = tamCS
    vmx->memoria.tabla_segmentos[0] = (uint32_t)tamCS & 0xFFFF;

    // Entrada 1 (Datos): Base = tamCS, Tamano = 16384 - tamCS
    vmx->memoria.tabla_segmentos[1] = (((uint32_t)tamCS & 0xFFFF) << 16) | ((uint32_t)(16384 - tamCS) & 0xFFFF);

    // Entradas 2 a 7: Invalidas (0xFFFFFFFF)
    for (i = 2; i < 8; i++) {
        vmx->memoria.tabla_segmentos[i] = 0xFFFFFFFF;
    }

    // Inicializa los registros punteros segun la especificacion:
    // CS = Segmento 0, Offset 0 -> 0x00000000
    // DS = Segmento 1, Offset 0 -> 0x00010000
    // IP = CS
    vmx->registros[CS] = 0x00000000;
    vmx->registros[DS] = 0x00010000;
    vmx->registros[IP] = vmx->registros[CS];

    logger("[CARGA] Archivo '%s' cargado con exito.\n", ruta_archivo);
    logger("[CARGA] Tamano del codigo: %d bytes.\n", tamCS);
    logger("[CARGA] Segmento Codigo (0): Base=0, Tamano=%d\n", tamCS);
    logger("[CARGA] Segmento Datos  (1): Base=%d, Tamano=%d\n", tamCS, 16384 - tamCS);
    logger("[CARGA] Registros iniciales: CS=0x%08X, DS=0x%08X, IP=0x%08X\n",
           vmx->registros[CS], vmx->registros[DS], vmx->registros[IP]);
}
