#include "vmx.h"
#include "decodificador.h"
#include "logger.h"
#include <stddef.h>

// Definicion de la tabla de despacho de operaciones (inicializada en NULL)
FuncionOperacion tabla_operaciones[32] = {NULL};

// Valida si el registro IP apunta dentro del segmento de codigo
int ip_valido(Vmx *vmx) {
    int segmento;
    int offset;
    int tamano_codigo;

    // Si IP es -1 (0xFFFFFFFF), finaliza la ejecucion normalmente (ejecuto STOP)
    if (vmx->registros[IP] == -1) {
        return 0;
    }

    // IP es una direccion logica: 16 bits altos = segmento, 16 bits bajos = offset
    // Lo que debemos validar es que el segmento sea cero en este caso, porque queremos acceder al codigo
    segmento = (vmx->registros[IP] >> 16) & 0xFFFF;
    offset = vmx->registros[IP] & 0xFFFF;

    // El codigo del proceso reside exclusivamente en el segmento 0
    if (segmento != 0) {
        return 0;
    }

    // El tamano del codigo esta almacenado en los 16 bits bajos de tabla_segmentos[0]
    tamano_codigo = (int)(vmx->tabla_segmentos[0] & 0xFFFF);

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
        vmx->abortar("Error: Instruccion invalida o codigo de operacion inexistente.");
        return;
    }

    // Fijar directo a la operacion correspondiente
    tabla_operaciones[opc](vmx);
}

// Ciclo principal de ejecucion: Fetch-Decode-Execute
void ejecutar_vmx(Vmx *vmx) {
    logger("[VMX] Iniciando ciclo de ejecucion.\n");

    while (ip_valido(vmx)) {
        // 1. Fetch y Decode: lee la instruccion en IP, carga OPC, OP1, OP2 y avanza IP
        decodificar_instruccion(vmx);

        // 2. Dispatch / Execute: ejecuta la operacion cargada en OPC
        ejecutar_instruccion(vmx);
    }

    logger("[VMX] Ciclo de ejecucion finalizado.\n");
}
