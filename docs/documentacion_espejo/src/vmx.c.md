## int ip_valido(Vmx \*vmx) {

Esta funcion valida que el ip sea valido, puede no ser valido al encontrar un STOP o por un error.

```c
    if (vmx->registros[IP] == -1) {
        logger("[VMX] Ciclo detenido: IP = -1 (STOP detectado).\n");
        return 0;
    }
```

Si la IP es -1 terminamos la ejecucion, retornamos 0 y quien llama a la funcion se encarga de cortar el programa.

```c
    segmento = (vmx->registros[IP] >> 16) & 0xFFFF;
    offset = vmx->registros[IP] & 0xFFFF;

    // El codigo del proceso reside exclusivamente en el segmento 0
    if (segmento != 0) {
        logger("[ERROR][VMX] IP invalido: segmento=%d (el codigo solo reside en segmento 0). IP=0x%08X\n",
               segmento, vmx->registros[IP]);
        return 0;
    }
```

Validamos en el registro IP de los registros, el valor del ip, asi como el LAR y demas cosas, en los 2 bytes altos se guarda el segmento y en los bajos el desplazamiento en este caso.

Si el segmento no es cero, debemos abortar porque el segmento debe ser el cero.

```c
    // El tamano del codigo esta almacenado en los 16 bits bajos de tabla_segmentos[0]
    tamano_codigo = (int)(vmx->memoria.tabla_segmentos[0] & 0xFFFF);

    // Valida que el offset este estrictamente dentro de los limites del segmento de codigo
    if (offset < 0 || offset >= tamano_codigo) {
        logger("[VMX] Ciclo detenido: IP fuera de los limites del segmento de codigo (offset=%d, tamano_codigo=%d). IP=0x%08X\n",
               offset, tamano_codigo, vmx->registros[IP]);
        return 0;
    }
```

Si la base mas el offset exede el tamaño del segmento, tambien debemos cortar, es un error.

### Ejecutar Instruccion

```c
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
```

Para ejecutar una instruccion, nos fijamos el registro opc, validamos que sea valido y luego directamente ejecutamos la funcion asociada al codigo, desde el array de punteros a funciones.

## Ciclo principal

```c
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
```

Mientras la instruccion siga siendo valida, vamos decodificando desde la memoria y vamos ejecutando cada instruccion.
