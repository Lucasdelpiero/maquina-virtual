#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "src/vmx.h"
#include "src/logger.h"
#include "src/disassembler.h"
#include "src/vmi.h"
#include "src/param_segment.h"

#define MAX_PARAMS 50
#define MAX_PARAM_LEN 100

// Verifica si el nombre del archivo termina con la extension requerida .vmx
static int tiene_extension_vmx(char nombre[]) {
    int len;
    if (nombre == NULL) return 0;
    len = strlen(nombre);
    if (len < 4) return 0;
    return (strcmp(&nombre[len - 4], ".vmx") == 0);
}

// Verifica si el nombre del archivo termina con la extension requerida .vmi
static int tiene_extension_vmi(char nombre[]) {
    int len;
    if (nombre == NULL) return 0;
    len = strlen(nombre);
    if (len < 4) return 0;
    return (strcmp(&nombre[len - 4], ".vmi") == 0);
}

// Parsea de forma secuencial los argumentos de la linea de comandos:
// vmx [filename.vmx] [filename.vmi] [m=M] [-d] [-p param1 param2 ... paramN] [-dev]
static int parsear_argumentos(int argc, char *argv[], char vmx_arch[], char vmi_arch[], int *mem, char params[MAX_PARAMS][MAX_PARAM_LEN], int *nro_params, int *modo_debug, int *modo_disassembler) {
    int pos = 1;
    int m_val;

    // Se utiliza fprintf en stderr para mensajes de error
    // El primer argumento es el nombre del (vmx), es decir el ejecutable de la maquina virtual, por ello empiezo con pos = 1
    if (argc < 2) {
        fprintf(stderr, "ERROR: Faltan argumentos requeridos\n");
        fprintf(stderr, "Uso: vmx [filename.vmx] [filename.vmi] [m=M] [-d] [-p param1 ... paramN] [-dev]\n");
        return 0;
    }

    // 1. Lectura de nombres de archivos (.vmx y/o .vmi) al inicio
    if (pos < argc && tiene_extension_vmx(argv[pos])) {
        strcpy(vmx_arch, argv[pos]);
        pos++;
        // Si vino .vmx, a continuacion puede venir opcionalmente .vmi
        if (pos < argc && tiene_extension_vmi(argv[pos])) {
            strcpy(vmi_arch, argv[pos]);
            pos++;
        }
    } else if (pos < argc && tiene_extension_vmi(argv[pos])) {
        // En caso de que solo se provea archivo de imagen .vmi
        strcpy(vmi_arch, argv[pos]);
        pos++;
    }

    // Validacion temprana: es obligatorio al menos uno de los dos archivos
    if (strlen(vmx_arch) == 0 && strlen(vmi_arch) == 0) {
        fprintf(stderr, "ERROR: Debe especificarse al menos un archivo (.vmx y/o .vmi) al inicio\n");
        fprintf(stderr, "Uso: vmx [filename.vmx] [filename.vmi] [m=M] [-d] [-p param1 ... paramN] [-dev]\n");
        return 0;
    }

    // 2. Lectura opcional de parametro de memoria m=M
    if (pos < argc && strncmp(argv[pos], "m=", 2) == 0) {
        m_val = atoi(&argv[pos][2]);
        if (m_val <= 0) {
            fprintf(stderr, "ERROR: Tamano de memoria invalido: '%s'\n", argv[pos]);
            return 0;
        }
        *mem = m_val;
        pos++;
    }

    // 3. Lectura opcional de bandera disassembler -d
    if (pos < argc && strcmp(argv[pos], "-d") == 0) {
        *modo_disassembler = 1;
        pos++;
    }

    // 4. Lectura opcional de bandera -dev si aparece antes de -p
    if (pos < argc && strcmp(argv[pos], "-dev") == 0) {
        *modo_debug = 1;
        pos++;
    }

    // 5. Lectura opcional de parametros para la subrutina principal: -p param1 ... paramN
    if (pos < argc && strcmp(argv[pos], "-p") == 0) {
        pos++;
        while (pos < argc) {
            // Si el ultimo argumento es -dev se asigna al modo debug
            if (strcmp(argv[pos], "-dev") == 0 && pos == argc - 1) {
                *modo_debug = 1;
            } else {
                if (*nro_params < MAX_PARAMS) {
                    strcpy(params[*nro_params], argv[pos]);
                    (*nro_params)++;
                } else {
                    fprintf(stderr, "ADVERTENCIA: Se supero el limite maximo de parametros (%d)\n", MAX_PARAMS);
                }
            }
            pos++;
        }
    }

    // 6. Lectura opcional de bandera -dev si vino al final y no se uso -p
    if (pos < argc && strcmp(argv[pos], "-dev") == 0) {
        *modo_debug = 1;
        pos++;
    }

    // 7. Si quedaron argumentos no procesados, se informa error de sintaxis
    if (pos < argc) {
        fprintf(stderr, "ERROR: Argumento inesperado o fuera de orden: '%s'\n", argv[pos]);
        fprintf(stderr, "Uso: vmx [filename.vmx] [filename.vmi] [m=M] [-d] [-p param1 ... paramN] [-dev]\n");
        return 0;
    }

    // Regla de especificacion: si no hay archivo .vmx, se ignoran los parametros -p
    if (strlen(vmx_arch) == 0 && *nro_params > 0) {
        *nro_params = 0;
    }

    // Valor por defecto para la memoria: 16 KiB
    if (*mem == -1) {
        *mem = 16;
    }

    return 1;
}

int main(int argc, char *argv[]) {
    char vmx_arch[260] = "";
    char vmi_arch[260] = "";
    int mem = -1;
    char params[MAX_PARAMS][MAX_PARAM_LEN];
    int nro_params = 0;
    int modo_debug = 0;
    int modo_disassembler = 0;
    Vmx vmx;
    int i;

    if (!parsear_argumentos(argc, argv, vmx_arch, vmi_arch, &mem, params, &nro_params, &modo_debug, &modo_disassembler)) {
        return 1;
    }

    inicializar_vmx(&vmx, modo_debug, modo_disassembler);

    logger("[MAIN] Parametros configurados:\n");
    logger("[MAIN]   Archivo VMX: '%s'\n", strlen(vmx_arch) > 0 ? vmx_arch : "(ninguno)");
    logger("[MAIN]   Archivo VMI: '%s'\n", strlen(vmi_arch) > 0 ? vmi_arch : "(ninguno)");
    logger("[MAIN]   Memoria: %d KiB\n", mem);
    logger("[MAIN]   Disassembler: %s\n", modo_disassembler ? "SI" : "NO");
    logger("[MAIN]   Debug: %s\n", modo_debug ? "SI" : "NO");
    logger("[MAIN]   Cantidad de parametros: %d\n", nro_params);
    for (i = 0; i < nro_params; i++) {
        logger("[MAIN]     Parametro %d: '%s'\n", i, params[i]);
    }

    vmx.memoria.tam_mem_kib = mem;
    if (strlen(vmi_arch) > 0) {
        vmi_configurar_archivo(vmi_arch);
    }

    // Carga inicial segun el tipo de archivo recibido
    if (strlen(vmx_arch) > 0) {
        InfoParamSegment info_ps;
        info_ps = construir_param_segment(&vmx, vmx_arch, params, nro_params);
        // TODO:
        (void)info_ps; // Conectar aca con la parte de Lucas

        cargar_programa(&vmx, vmx_arch);
    } else {
        if (!cargar_imagen_vmi(&vmx, vmi_arch, &mem)) {
            fprintf(stderr, "ERROR: No se pudo cargar el archivo de imagen '%s'\n", vmi_arch);
            return 1;
        }
        vmx.memoria.tam_mem_kib = mem;
    }

    // El disassembler se ejecuta siempre que este activo el flag -d, sobre la memoria ya cargada
    if (modo_disassembler) {
        desensamblar_programa(&vmx);
    }

    ejecutar_vmx(&vmx);

    logger("[MAIN] Proceso finalizado exitosamente.\n");

    return 0;
}
