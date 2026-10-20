#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "depurador.h"
#include "vmi.h"
#include "logger.h"

// Banderas de control de depuracion
static int modo_paso_a_paso = 0;
static int fue_breakpoint = 0;

void depurador_manejar_breakpoint(Vmx *vmx) {
    char linea[16];
    char opcion;

    if (vmx == NULL) {
        return;
    }

    // Especificacion de la vm: los breakpoints se ignoran si no se incluyo archivo .vmi
    if (!vmi_tiene_archivo()) {
        logger("[DEBUG] Breakpoint alcanzado pero ignorado (no se especifico archivo .vmi).\n");
        return;
    }

    // Marca que esta instruccion ya disparo una pausa de breakpoint
    fue_breakpoint = 1;

    // 1. Pausa y genera la imagen con el estado actual
    guardar_imagen_vmi(vmx, NULL, 0);

    printf("[BREAKPOINT] Pausa en IP = 0x%08X | Imagen actualizada en '%s'\n",
           vmx->registros[IP], vmi_obtener_archivo());

    // 2. Bucle de espera de accion del usuario
    while (1) {
        printf("[BREAKPOINT] Accion [g = continuar | q = salir | Enter = paso a paso]:\n");

        if (fgets(linea, sizeof(linea), stdin) == NULL) {
            printf("\n");
            exit(0);
        }

        // Si presiono unicamente la tecla Enter (paso a paso)
        if (linea[0] == '\n' || linea[0] == '\r') {
            modo_paso_a_paso = 1;
            logger("[BREAKPOINT] Ejecutando siguiente instruccion (paso a paso)...\n");
            break;
        }

        opcion = linea[0];

        // Opcion 'g' o 'G': continuar normalmente hasta el proximo breakpoint o fin
        if (opcion == 'g' || opcion == 'G') {
            modo_paso_a_paso = 0;
            logger("[BREAKPOINT] Reanudando ejecucion normal...\n");
            break;
        }

        // Opcion 'q' o 'Q': abortar ejecucion dejando la imagen .vmi intacta
        if (opcion == 'q' || opcion == 'Q') {
            printf("[BREAKPOINT] Ejecucion abortada por el usuario. La imagen '%s' permanece intacta.\n",
                   vmi_obtener_archivo());
            exit(0);
        }

        printf("[BREAKPOINT] Opcion '%c' no reconocida. Ingrese 'g', 'q' o presione Enter.\n", opcion);
    }
}

void depurador_verificar_paso_a_paso(Vmx *vmx) {
    // Si la instruccion actual ya pauso por un breakpoint explicito, no se repite la pausa
    if (fue_breakpoint) {
        fue_breakpoint = 0;
        return;
    }

    // Si esta activo el modo paso a paso, pausa tras ejecutar la instruccion
    if (modo_paso_a_paso) {
        depurador_manejar_breakpoint(vmx);
        fue_breakpoint = 0;
    }
}
