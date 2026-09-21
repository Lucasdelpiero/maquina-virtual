### Leer valor de operando

```c
// Lee un operando de la memoria fisica del segmento de codigo
static int32_t leer_operando(Vmx *vmx, int tipo, int *pos) {
    int32_t dato = 0;
    int p = *pos;
```

Los pasos para leer un operando del code segment y guardarlo en los registros OP1/OP2 son:

Primero que nada tenemos como parametro el tipo, ejecutamos diferente logica segun el tipo:

```c
    if (tipo == TIPO_REGISTRO) {
        // 1 byte: codigo de registro en los 5 bits menos significativos
        dato = (int32_t)(vmx->memoria.mem_principal[p] & 0x1F);
        *pos = p + 1;
    } else if (tipo == TIPO_INMEDIATO) {
        // 2 bytes big-endian: valor entero de 16 bits
        uint16_t val16 = ((uint16_t)vmx->memoria.mem_principal[p] << 8) | (uint16_t)vmx->memoria.mem_principal[p + 1];
        dato = (int32_t)val16;
        *pos = p + 2;
    } else if (tipo == TIPO_MEMORIA) {
        // 3 bytes: 2 bytes de offset big-endian + 1 byte con codigo de registro (5 bits bajos)
        uint16_t offset = ((uint16_t)vmx->memoria.mem_principal[p] << 8) | (uint16_t)vmx->memoria.mem_principal[p + 1];
        uint8_t cod_reg = vmx->memoria.mem_principal[p + 2] & 0x1F;
        dato = ((int32_t)offset << 8) | (int32_t)cod_reg;
        *pos = p + 3;
    }
    return dato;
```

Segun el tipo del operando, extraemos el valor del operando leyendo de la memoria de diferentes formas.

### Decodificar Instruccion

```c
    // IP es una direccion logica relativa al segmento de codigo (segmento 0)
    offset_ip = vmx->registros[IP] & 0xFFFF;
    tam_codigo = (int)(vmx->memoria.tabla_segmentos[0] & 0xFFFF);

    if (offset_ip < 0 || offset_ip >= tam_codigo) {
        logger("[ERROR][DECODER] Intento de decodificar fuera del segmento de codigo: offset_ip=%d, tam_codigo=%d, IP=0x%08X\n",
               offset_ip, tam_codigo, vmx->registros[IP]);
        vmx->abortar("Error: Intento de decodificar instruccion fuera del segmento de codigo.");
    }
```

Validamos la ip que toca procesar primero que nada, puede ser mayor al tamaño del codigo y debemos abortar.

Luego:

El 1er byte es la de instruccion, asi que lo miramos y diferenciamos si es una operacion con 0, 1 o 2 operandos

```c
    if (primer_byte == OP_STOP) {
        // Instruccion sin operandos (STOP, 0x0F)
        opc = OP_STOP;
        tipo_a = TIPO_NINGUNO;
        tipo_b = TIPO_NINGUNO;
    } else if (((primer_byte >> 4) & 0x03) == 0) {
        // Instruccion con un solo operando (0x00 a 0x0A: SYS, JMP, Jcc, NOT)
        opc = primer_byte & 0x1F;
        tipo_a = (primer_byte >> 6) & 0x03;
        tipo_b = TIPO_NINGUNO;

        // Lee el operando A
        dato_a = leer_operando(vmx, tipo_a, &pos);
    } else {
        // Instruccion con dos operandos (0x10 a 0x1F: MOV, ADD, SUB, ...)
        opc = 0x10 | (primer_byte & 0x0F);
        tipo_b = (primer_byte >> 6) & 0x03;
        tipo_a = (primer_byte >> 4) & 0x03;

        // En el binario se codifica primero el operando B y luego el A
        dato_b = leer_operando(vmx, tipo_b, &pos);
        dato_a = leer_operando(vmx, tipo_a, &pos);
    }
```

Dependiendo el tipo de **codigo de operacion** leemos los tipos de los operandos y luego sus valores (la lectura depende de sus tipos)

El codigo de operacion y los tipos de los operandos estan en el **byte de cabecera**

`tam_instruccion = pos - offset_ip;`

La posicion, pos es donde estamos parados en la memoria actualmente, el offset es la posicion de la instruccion que estamos ejecutando ahora mismo.

Y el tamaño entero de la instruccion, que es variable se halla restando la posicion final una vez leido los dos valores de los operandos y el offset (donde comienza la cabecera o donde apuntaba ip+1)

```c
    // Empaqueta en registros OPC, OP1 y OP2
    vmx->registros[OPC] = opc;
    vmx->registros[OP1] = (tipo_a << 24) | (dato_a & 0x00FFFFFF);
    vmx->registros[OP2] = (tipo_b << 24) | (dato_b & 0x00FFFFFF);

    // Avanza el registro IP sumando los bytes consumidos por la instruccion
    vmx->registros[IP] += tam_instruccion;

    logger("[DECODER] IP_prev=0x%04X Opcode=0x%02X (%d bytes) OP1=(tipo=%d dato=0x%X) OP2=(tipo=%d dato=0x%X) -> IP_nuevo=0x%08X\n",
           offset_ip, opc, tam_instruccion, tipo_a, dato_a, tipo_b, dato_b, vmx->registros[IP]);
```

Guardamos los registros de los operandos, y actualizamos el registro del codigo de operacion.
Avanzamos el registro IP sumando toda la instruccion leida para la lectura de la siguiente ejecucion y el proceso se vuelve a ejecutar luego.