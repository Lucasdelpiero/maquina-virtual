#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "src/vmx.h"

// Verifica si el nombre del archivo termina con la extension requerida .vmx
static int tiene_extension_vmx(char nombre[]) {
    int len;
    if (nombre == NULL) return 0;
    len = strlen(nombre);
    if (len < 4) return 0;
    return (strcmp(&nombre[len - 4], ".vmx") == 0);
}

// Parsea los argumentos de la linea de comandos
// Sintaxis esperada: vmx <archivo.vmx> [-d] [debug]
static int parsear_argumentos(int argc, char *argv[], char nombre_arch[], int *modo_debug, int *modo_disassembler) {
    int i;

    if (argc < 2) {
        printf("ERROR: Falta el nombre del archivo\n");
        printf("Uso: vmx <archivo.vmx> [-d] [debug]\n");
        return 0;
    }

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            *modo_disassembler = 1;
        } else if (strcmp(argv[i], "debug") == 0) {
            *modo_debug = 1;
        } else if (argv[i][0] == '-') {
            printf("ERROR: Opcion no reconocida '%s'\n", argv[i]);
            printf("Uso: vmx <archivo.vmx> [-d] [debug]\n");
            return 0;
        } else {
            if (strlen(nombre_arch) > 0) {
                printf("ERROR: Se especifico mas de un archivo: '%s' y '%s'\n", nombre_arch, argv[i]);
                return 0;
            }
            if (!tiene_extension_vmx(argv[i])) {
                printf("ERROR: El archivo '%s' debe tener extension .vmx\n", argv[i]);
                return 0;
            }
            strcpy(nombre_arch, argv[i]);
        }
    }

    if (strlen(nombre_arch) == 0) {
        printf("ERROR: No se encontro el nombre del archivo\n");
        printf("Uso: vmx <archivo.vmx> [-d] [debug]\n");
        return 0;
    }

    return 1;
}

int main(int argc, char *argv[]) {
    char nombre_arch[50] = "";
    int modo_debug = 0;
    int modo_disassembler = 0;
    Vmx vmx;

    if (!parsear_argumentos(argc, argv, nombre_arch, &modo_debug, &modo_disassembler)) {
        return 1;
    }

    inicializar_vmx(&vmx, modo_debug, modo_disassembler);
    cargar_programa(&vmx, nombre_arch);
    ejecutar_vmx(&vmx);

    return 0;
}
