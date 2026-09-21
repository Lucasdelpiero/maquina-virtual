#include "vmx.h"
#include "decodificador.h"
#include "disassembler.h"
#include "logger.h"
#include <stdio.h>
#include <stddef.h>

// Definicion de la tabla de operaciones (inicializada en NULL)
FuncionOperacion tabla_operaciones[32] = {NULL};

// Valida si el registro IP apunta dentro del segmento de codigo
int ip_valido(Vmx *vmx) {
    int segmento;
    int offset;
    int tamano_codigo;

    // Si IP es -1 (0xFFFFFFFF), finaliza la ejecucion normalmente (ejecuto STOP)
    if (vmx->registros[IP] == -1) {
        logger("[VMX] Ciclo detenido: IP = -1 (STOP detectado).\n");
        return 0;
    }

    // IP es una direccion logica: 16 bits altos = segmento, 16 bits bajos = offset
    // Lo que debemos validar es que el segmento sea cero en este caso, porque queremos acceder al codigo
    segmento = (vmx->registros[IP] >> 16) & 0xFFFF;
    offset = vmx->registros[IP] & 0xFFFF;

    // El codigo del proceso reside exclusivamente en el segmento 0
    if (segmento != 0) {
        logger("[ERROR][VMX] IP invalido: segmento=%d (el codigo solo reside en segmento 0). IP=0x%08X\n",
               segmento, vmx->registros[IP]);
        return 0;
    }

    // El tamano del codigo esta almacenado en los 16 bits bajos de tabla_segmentos[0]
    tamano_codigo = vmx->memoria.tabla_segmentos[0] & 0xFFFF;

    /*
        IP = 0x0000 0004
            │       │
      ┌─────┘       └────────┐
      ▼                      ▼
    Segmento = 0            Offset = 4
      │
      ▼ (Busca en tabla)
    tabla_segmentos[0] ───► Base = 0, Límite = 100
                         │
                         ▼
        Dirección Física = Base + Offset
                         = 0 + 4
                         = 4  ──► memoria[4] (Próxima instrucción)
    */
    // Valida que el offset este estrictamente dentro de los limites del segmento de codigo
    if (offset < 0 || offset >= tamano_codigo) {
        logger("[VMX] Ciclo detenido: IP fuera de los limites del segmento de codigo (offset=%d, tamano_codigo=%d). IP=0x%08X\n",
               offset, tamano_codigo, vmx->registros[IP]);
        return 0;
    }

    return 1;
}

// Fija la instruccion cargada en el registro OPC
void ejecutar_instruccion(Vmx *vmx) {
    int opc;

    opc = vmx->registros[OPC];

    // Verifica que el codigo de operacion este dentro de los limites y que la operacion exista
    if (opc < 0 || opc >= 32 || tabla_operaciones[opc] == NULL) {
        logger("[ERROR][VMX] Instruccion invalida o no implementada: OPC=0x%02X (%d) en IP=0x%08X\n",
               opc, opc, vmx->registros[IP]);
        vmx->abortar("Error: Instruccion invalida o codigo de operacion inexistente.");
        return;
    }

    logger("[VMX] Delegando operacion: %s (OPC=0x%02X) [IP=0x%08X]\n",
           obtener_mnemonico(opc), opc, vmx->registros[IP]);

    // Fijar directo a la operacion correspondiente
    tabla_operaciones[opc](vmx);
}

// Ciclo principal de ejecucion: busqueda, decodificacion y ejecucion
void ejecutar_vmx(Vmx *vmx) {
    logger("[VMX] Iniciando ciclo de ejecucion.\n");

    while (ip_valido(vmx)) {
        // 1. Busqueda y Decodificacion: lee la instruccion en IP, carga OPC, OP1, OP2 y avanza IP
        decodificar_instruccion(vmx);

        // 2. Ejecucion: delega la operacion cargada en OPC
        ejecutar_instruccion(vmx);
    }

    logger("[VMX] Ciclo de ejecucion finalizado.\n");
}
