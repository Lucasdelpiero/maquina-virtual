#include "vmx.h"
#include "errores.h"
#include "logger.h"
#include <stdio.h>
#include <stddef.h>

void inicializar_vmx(Vmx *vmx, int modo_debug, int modo_disassembler) {
    int i;
    vmx->modo_debug = modo_debug;
    vmx->modo_disassembler = modo_disassembler;
    vmx->abortar = abortar;

    logger_habilitar(modo_debug);

    // Inicializa los 32 registros en cero
    for (i = 0; i < 32; i++) {
        vmx->registros[i] = 0;
    }

    // Inicializa la memoria principal en cero
    for (i = 0; i < 16384; i++) {
        vmx->memoria[i] = 0;
    }

    // Inicializa las entradas de la tabla de segmentos en invalido (0xFFFFFFFF)
    // Antes de leer el archivo se inicializa como segmentos invalidos, luego se inicializan correctamente
    // Esto tiene la ventaja de que el programa crashea rapido al intentar usar/acceder a un segmento invalido
    for (i = 0; i < 8; i++) {
        vmx->tabla_segmentos[i] = 0xFFFFFFFF;
    }

    // Inicializa la tabla de despacho de operaciones en NULL
    for (i = 0; i < 32; i++) {
        tabla_operaciones[i] = NULL;
    }
}

void cargar_programa(Vmx *vmx, char ruta_archivo[]) {
    FILE *arch;
    unsigned char cabecera[8];
    int leidos;
    int tamano_codigo;
    int bytes_leidos;

    arch = fopen(ruta_archivo, "rb");
    if (arch == NULL) {
        vmx->abortar("Error: No se pudo abrir el archivo .vmx especificado.");
    }

    // Lee los 8 bytes de cabecera
    leidos = fread(cabecera, 1, 8, arch);
    if (leidos != 8) {
        fclose(arch);
        vmx->abortar("Error: No se pudo leer la cabecera completa del archivo .vmx.");
    }

    // Valida identificador "VMX26" (bytes 0 a 4)
    if (cabecera[0] != 'V' || cabecera[1] != 'M' || cabecera[2] != 'X' ||
        cabecera[3] != '2' || cabecera[4] != '6') {
        fclose(arch);
        vmx->abortar("Error: Identificador de cabecera invalido. Se esperaba 'VMX26'.");
    }

    // Valida version (byte 5)
    if (cabecera[5] != 1) {
        fclose(arch);
        vmx->abortar("Error: Version de archivo .vmx no compatible. Se esperaba version 1.");
    }

    // Extrae el tamaño del codigo (bytes 6 y 7 en formato big-endian)
    // 0000 0000 0000 0100 --> 00 04  (tamaño 4 bytes por ej)
    // 0000 0000 << 8 = 0000 0000 0000 0000
    // 0000 0000 0000 0000 | 0000 0100 = 0000 0000 0000 0100
    // Requerido, es necesario castear a int antes de hacer shift a la izquierda, porque el char es de 8 bits
    tamano_codigo = ((int)cabecera[6] << 8) | (int)cabecera[7];

    // Valida que el tamaño del codigo este dentro del rango permitido (1 a 16384 bytes)
    if (tamano_codigo <= 0 || tamano_codigo > 16384) {
        fclose(arch);
        vmx->abortar("Error: Tamano de codigo invalido en la cabecera del archivo .vmx.");
    }

    // Carga el codigo del programa en memoria a partir de la posicion 0
    bytes_leidos = fread(&vmx->memoria[0], 1, tamano_codigo, arch);
    if (bytes_leidos != tamano_codigo) {
        fclose(arch);
        vmx->abortar("Error: El archivo .vmx termino antes de lo esperado o esta danado.");
    }

    fclose(arch);

    // Configura la tabla de descriptores de segmentos:
    // Entrada 0 (Codigo): Base = 0, Tamano = tamano_codigo
    vmx->tabla_segmentos[0] = (uint32_t)tamano_codigo;

    // Entrada 1 (Datos): Base = tamano_codigo, Tamano = 16384 - tamano_codigo
    // Porque se hace esto?. Debemos castear el tamano_codigo que es de 4 bytes a 32 bits del int32_t
    // Pero al castear se añaden ceros adelante que debo eliminar, porque casteo y luego el shift
    // La tabla de segmentos en 1 queda con la informacion real: tamano_codigo, tamaño restante.
    // Al final quedan dos partes de 16 bits que se concatenan
    vmx->tabla_segmentos[1] = (((uint32_t)tamano_codigo & 0xFFFF) << 16) | ((uint32_t)(16384 - tamano_codigo) & 0xFFFF);

    // Inicializa los registros puntero en direcciones logicas:
    // CS = Segmento 0, Offset 0 -> 0x00000000
    // DS = Segmento 1, Offset 0 -> 0x00010000
    // IP = Puntero a la primera instruccion (mismo valor inicial que CS)
    vmx->registros[CS] = 0x00000000;
    vmx->registros[DS] = 0x00010000;
    vmx->registros[IP] = vmx->registros[CS];

    logger("[CARGA] Archivo '%s' cargado con exito.\n", ruta_archivo);
    logger("[CARGA] Tamano del codigo: %d bytes.\n", tamano_codigo);
    logger("[CARGA] Segmento Codigo (0): Base=0, Tamano=%d\n", tamano_codigo);
    logger("[CARGA] Segmento Datos  (1): Base=%d, Tamano=%d\n", tamano_codigo, 16384 - tamano_codigo);
    logger("[CARGA] Registros iniciales: CS=0x%08X, DS=0x%08X, IP=0x%08X\n",
           vmx->registros[CS], vmx->registros[DS], vmx->registros[IP]);
}
