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
    int inicio_cs = 0;
    int fin_cs = tamCS;
    int inicio_ds = fin_cs;
    int fin_ds = TAM_MEMORIA_PRINCIPAL - fin_cs; // Es en realidad el tamaño del data segment

    mem->tabla_segmentos[0] = ((uint32_t)inicio_cs << 16) | (fin_cs & 0xFFFF);
    mem->tabla_segmentos[1] = ((uint32_t)inicio_ds << 16) | (fin_ds & 0xFFFF);

    int i;
    for (i = 2; i < TAM_SEGMENTOS; i++) {
        mem->tabla_segmentos[i] = 0xFFFFFFFF;
    }
}

void inicializar_vmx(Vmx *vmx, int modo_debug, int modo_disassembler) {
    int i;
    vmx->modo_debug = modo_debug;
    vmx->modo_disassembler = modo_disassembler;
    vmx->abortar = abortar; //abortar esta importada desde errors.c

    logger_habilitar(modo_debug);
    logger("[INIT] Inicializando VMX: modo_debug=%d, modo_disassembler=%d.\n", modo_debug, modo_disassembler);

    // Inicializa los 32 registros en cero
    for (i = 0; i < 32; i++) {
        vmx->registros[i] = 0;
    }

    // Inicializa la memoria principal y tabla de descriptores
    inicializar_memoria(&vmx->memoria);
    vmx->memoria.registros = (uint32_t *)vmx->registros;

    // Inicializa la tabla de operaciones
    inicializar_operadores();
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

void cargar_programa(Vmx *vmx, char ruta_archivo[], int tam_parametros) {
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
    if (version != 1 && version != 2) {
        fclose(f);
        logger("[ERROR][CARGA] Version %d no compatible (se esperaba 1 o 2) en '%s'\n", version, ruta_archivo);
        vmx->abortar("Error: Version de archivo .vmx no compatible. Se esperaba version 1 o 2.");
    }

    if (version == 1) {
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
        vmx->memoria.tabla_segmentos[0] = tamCS & 0xFFFF;

        // Entrada 1 (Datos): Base = tamCS, Tamano = 16384 - tamCS
        vmx->memoria.tabla_segmentos[1] = ((uint32_t)tamCS << 16) | (16384 - tamCS);

        // Entradas 2 a 7: Invalidas (0xFFFFFFFF)
        for (i = 2; i < 8; i++) {
            vmx->memoria.tabla_segmentos[i] = 0xFFFFFFFF;
        }

        // Inicializa los registros punteros segun la especificacion:
        // CS = Segmento 0, Offset 0 -> 0x00000000
        // DS = Segmento 1, Offset 0 -> 0x00010000
        // IP = CS
        vmx->registros[CS] = 0x00000000;
        vmx->registros[DS] = 0x00010000; // Este 1 es porque el DS es un puntero y apunta en los primeros 16 bits al 1er segmento
        vmx->registros[IP] = vmx->registros[CS];

        logger("[CARGA] Archivo '%s' cargado con exito.\n", ruta_archivo);
        logger("[CARGA] Tamano del codigo: %d bytes.\n", tamCS);
        logger("[CARGA] Segmento Codigo (0): Base=0, Tamano=%d\n", tamCS);
        logger("[CARGA] Segmento Datos  (1): Base=%d, Tamano=%d\n", tamCS, 16384 - tamCS);
        logger("[CARGA] Registros iniciales: CS=0x%08X, DS=0x%08X, IP=0x%08X\n",
               vmx->registros[CS], vmx->registros[DS], vmx->registros[IP]);
    } else {
        // Cabecera Version 2: 18 bytes totales (6 campos de 2 bytes big-endian)
        int tam_codigo = 0;
        int tam_datos = 0;
        int tam_extra = 0;
        int tam_pila = 0;
        int tam_const = 0;
        int offset_entry_point = 0;
        int tam_ps = 0;
        int pos_const = 0;
        int pos_codigo = 0;
        int pos_fin_codigo = 0;
        int limite_memoria = 0;

        if (!leer_uint16_be(f, &tam_codigo) ||
            !leer_uint16_be(f, &tam_datos)  ||
            !leer_uint16_be(f, &tam_extra)  ||
            !leer_uint16_be(f, &tam_pila)   ||
            !leer_uint16_be(f, &tam_const)  ||
            !leer_uint16_be(f, &offset_entry_point)) {
            fclose(f);
            logger("[ERROR][CARGA] Fallo al leer campos de cabecera V2 en '%s'\n", ruta_archivo);
            vmx->abortar("Error: No se pudo leer la cabecera completa del archivo .vmx version 2.");
        }

        if (tam_codigo <= 0) {
            fclose(f);
            logger("[ERROR][CARGA] Tamano de codigo invalido en V2: %d bytes\n", tam_codigo);
            vmx->abortar("Error: Tamano de codigo invalido en la cabecera del archivo .vmx.");
        }

        // Si el registro PS no es -1 (0xFFFFFFFF) y la entrada 0 esta asignada, existe Param Segment
        if (vmx->registros[PS] != -1 && vmx->memoria.tabla_segmentos[0] != 0xFFFFFFFF) {
            tam_ps = vmx->memoria.tabla_segmentos[0] & 0xFFFF;
        } else {
            tam_ps = 0;
        }

        // Organizacion de memoria fisica segun la parte 2 de la vm:
        // Param Segment -> Const Segment -> Code Segment -> Data Segment -> Extra Segment -> Stack Segment
        pos_const = tam_ps;
        pos_codigo = pos_const + tam_const;
        pos_fin_codigo = pos_codigo + tam_codigo;

        limite_memoria = vmx->memoria.tam_mem_kib * 1024;

        // Valida que el proceso entre en memoria
        // pos_fin_codigo = Param Segment + Const Segment + Code Segment lo que si estoy considerando toda la memoria
        if (pos_fin_codigo + tam_datos + tam_extra + tam_pila > limite_memoria) {
            fclose(f);
            logger("[ERROR][CARGA] Memoria insuficiente para alojar proceso (%d bytes requeridos > %d bytes disponibles)\n",
                   pos_fin_codigo + tam_datos + tam_extra + tam_pila, limite_memoria);
            vmx->abortar("Error: Memoria insuficiente.");
        }

        // En el archivo .vmx binario: primero va el codigo (tam_codigo) y luego Const Segment (tam_const)
        // 1. Leer Code Segment y escribir en RAM en pos_codigo (despues de PS y KS)
        i = 0;
        while (i < tam_codigo && fread(&byteLeido, sizeof(uint8_t), 1, f) == 1) {
            vmx->memoria.mem_principal[pos_codigo + i] = byteLeido;
            i++;
        }
        if (i != tam_codigo) {
            fclose(f);
            logger("[ERROR][CARGA] Archivo truncado al leer Code Segment (%d de %d bytes leidos)\n", i, tam_codigo);
            vmx->abortar("Error: El archivo .vmx termino antes de lo esperado o esta danado.");
        }

        // 2. Leer Const Segment y escribir en RAM en pos_const (despues de PS)
        if (tam_const > 0) {
            i = 0;
            while (i < tam_const && fread(&byteLeido, sizeof(uint8_t), 1, f) == 1) {
                vmx->memoria.mem_principal[pos_const + i] = byteLeido;
                i++;
            }
            if (i != tam_const) {
                fclose(f);
                logger("[ERROR][CARGA] Archivo truncado al leer Const Segment (%d de %d bytes leidos)\n", i, tam_const);
                vmx->abortar("Error: El archivo .vmx termino antes de lo esperado o esta danado.");
            }
        }

        fclose(f);

        crear_segmentos(&(vmx->memoria), tam_parametros, tam_const, tam_codigo, tam_datos, tam_extra, tam_pila);


        logger("[CARGA] Archivo V2 '%s' cargado con exito.\n", ruta_archivo);
        logger("[CARGA]   Header: Code=%d, Data=%d, Extra=%d, Stack=%d, Const=%d, EntryPoint=0x%04X\n",
               tam_codigo, tam_datos, tam_extra, tam_pila, tam_const, offset_entry_point);
        logger("[CARGA]   Distribucion en memoria fisica: Param=[0..%d], Const=[%d..%d], Code=[%d..%d]\n",
               tam_ps, pos_const, pos_const + tam_const, pos_codigo, pos_fin_codigo);
    }
}

// Agrega cantidad de argumentos y vector, ademas de un RET al stack segment
void inicializar_subrutina_principal(Memoria mem, InfoParamSegment info_ps){
    int dir_fisica = get_dir_fisica(mem, mem->registros[SP]);

    escribir_memoria(mem, dir_fisica, 4, info_ps.dir_argv); //  *argv a pila
    mem->registros[SP] -= 4;

    escribir_memoria(mem, dir_fisica, 4, info_ps.argc);     // *argc a pila
    mem->registros[SP] -= 4;

    escescribir_memoria(mem, dir_fisica, 4, -1);            // -1 a pila
}

