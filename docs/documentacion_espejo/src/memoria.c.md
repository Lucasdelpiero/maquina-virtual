```c
void set_registros_memoria(Memoria *mem, int32_t dir_logica, int cant_accedidos, int32_t dir_fisica, int32_t valor) {
    if (mem == NULL || mem->registros == NULL) {
        return;
    }
    // LAR = direccion logica accedida
    mem->registros[LAR] = dir_logica;

    // MAR = [CANT ACCEDIDOS  |   DIR_FISICA ]     [16 bits | 16 bits]
    mem->registros[MAR] = cant_accedidos;
    mem->registros[MAR] = mem->registros[MAR] << 16;
    mem->registros[MAR] += dir_fisica;

    // MBR = valor guardado/sacado
    mem->registros[MBR] = valor;
}
```

LAR guarda la memoria logica accedida y MAR guarda la direccion fisica accedida en los 4 bytes altos y la dir fisica accedida en los bytes bajos.

Aclarar que el `+` funciona como un OR en este caso, fijando la dir fisica en los bytes bajos.

## int32_t get_valor_memoria(Memoria *mem, int32_t dir_logica, int bytes_accedidos) {

```c
    // Concatena el valor segun la cantidad de bytes requeridos
    resultado = mem->mem_principal[dir_fisica];
    for (i = 1; i < bytes_accedidos; i++) {
        resultado = resultado << 8;
        resultado += mem->mem_principal[dir_fisica + i];
    }
```

Voy corriendo a izquierda y leyendo de a 8 bits, porque cada componente del array de memoria es de 8 bits.

## Porque usamos uint16?

```c
uint16_t traducir_direccion(Memoria *mem, int32_t dir_logica, uint16_t cant_bytes) {

    int segmento = (dir_logica >> 16) & 0xFFFF;
```

En traducir_direccion usamos uint 16 porque solo devolvemos la direccion.

Esto nos evita problemas con los signos, una direccion de memoria no podria ser negativa, asi para facilitarnos la vida, lo defininimos `uint` entonces al castear a int32 como hacemos en algunos lugares, no se hace la extension de signo.

Ademas nos permite meterlo directamente en el MAR, cuyos 4 bytes bajos ocupan justo 16 bits para la memoria fisica.

## void escribir_memoria

```c
void escribir_memoria(Memoria *mem, uint16_t dir_fisica, uint8_t cant_bytes, int32_t valor) {
    int bytes[4];
    int i;
    int offset = 0;
    int primer_byte;

    if (cant_bytes < 1 || cant_bytes > 4 || (int)dir_fisica + cant_bytes > TAM_MEMORIA_PRINCIPAL) {
        logger("[ERROR][MEMORIA] Escritura invalida: dir_fisica=0x%04X, cant_bytes=%d, valor=0x%08X\n",
               dir_fisica, cant_bytes, valor);
        abortar("Fallo de segmento: intento de escribir fuera de la memoria fisica.");
    }

    // Descompone el valor de 32 bits en bytes big-endian
    for (i = 0; i < 4; i++) {
        bytes[3 - i] = (valor >> (8 * i)) & 0xFF;
    }

    primer_byte = 4 - cant_bytes;
    for (i = primer_byte; i < 4; i++) {
        mem->mem_principal[dir_fisica + offset] = (uint8_t)bytes[i];
        offset++;
    }

    logger("[MEMORIA] Escribir %d byte(s) en DirFisica=0x%04X <= valor=0x%08X (%d)\n",
           cant_bytes, dir_fisica, valor, valor);
}
```

Esta funcion es flexible, escribe a memoria (donde cada celda es un byte) solo la cant_bytes requerida.

El valor es de 32 bits, 4 bytes, osea que eso representa el tamaño maximo a escribir, sino hay que llamar mas de una vez a la funcion.

Si queremos escribir un solo byte, el valor estara en los 8 bits bajos del valor de 32 bits.