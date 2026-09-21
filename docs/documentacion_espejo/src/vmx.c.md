## int ip_valido(Vmx *vmx) {

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

