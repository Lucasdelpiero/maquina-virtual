## cargar_programa

### Carga para archivos de la parte 1

```c
    if (version == 1) {
        // Carga tamano de codigo big-endian (2 bytes)
        for (i = 0; i < 2; i++) {
    ...
```

```c
    tamCS = (temp[0] << 8) | temp[1];

    if (tamCS <= 0 || tamCS > TAM_MEMORIA_PRINCIPAL) {
        fclose(f);
        logger("[ERROR][CARGA] Tamano de codigo invalido: %d bytes (limite 1..%d) en '%s'\n", tamCS, TAM_MEMORIA_PRINCIPAL, archNom);
        abortar("Error: Tamano de codigo invalido en la cabecera.");
        return -1;
    }
```

Se guarda el tamaño del code segment usando 16 bits del `tamCS` que es de 32 bits.

Guardarlo como un `int` de 32 bits tiene la ventaja de que si un archivo `.vmx` trae un tamaño de código inválido (por ejemplo `40000` bytes, que excede los `16384` de la memoria), el valor se almacena limpiamente como un número positivo sin sufrir overflow. Si hubiéramos usado un entero con signo de 16 bits (`int16_t`), cualquier valor mayor a `32767` desbordaría y se convertiría en un número negativo, pudiendo falsear las comparaciones o mostrar mensajes de error confusos.

Y dado que solo hay 2 byes para indicar el tamaño del codigo, con estos dos no es posible meter un numero que supere el limite de los enteros de 32 bits.

Luego:

```c
    fclose(f);
    setear_tabla_segmento(mem, tamCS);
    return 0;
```

Luego 

```c
    vmx->memoria.tabla_segmentos[1] = ((uint32_t)tamCS << 16) | (16384 - tamCS);
```

Aqui el 1er segmento de la tabla de segmentos, corresponde al code segment, en los primeros 2 bytes va la base, y en los otros dos va tamaño

La base se pone haciendo el shift y es igual al tamaño del Code segment, porque empieza donde termina el code segment

Y el tamaño es igual al tamaño total de la memoria que es 16384 - el tamaño del cs logicamente, el exedecente es el tamaño del ds.

```c
    vmx->registros[CS] = 0x00000000;
    vmx->registros[DS] = 0x00010000;
    vmx->registros[IP] = vmx->registros[CS];
```

Este 1 es porque el DS es un puntero y apunta en los primeros 16 bits al 1er segmento.
0001 -> Primeros 4 bytes es el segmento
0000 -> Segmento

### Carga para archivos de la parte 2

```c
    } else {
        // Cabecera Version 2: 18 bytes totales (6 campos de 2 bytes big-endian)
```

Leer el tamaño para cada segmento:

```c
        if (!leer_uint16_be(f, &tam_codigo) ||
            !leer_uint16_be(f, &tam_datos)  ||
            !leer_uint16_be(f, &tam_extra)  ||
            !leer_uint16_be(f, &tam_pila)   ||
            !leer_uint16_be(f, &tam_const)  ||
            !leer_uint16_be(f, &offset_entry_point)) {
            fclose(f);
```

```c
        // Si el registro PS no es -1 (0xFFFFFFFF) y la entrada 0 esta asignada, existe Param Segment
        if (vmx->registros[PS] != -1 && vmx->memoria.tabla_segmentos[0] != 0xFFFFFFFF) {
            tam_ps = vmx->memoria.tabla_segmentos[0] & 0xFFFF;
        } else {
            tam_ps = 0;
        }
```

Esto si es valido porque el param segment se inicializa en main

Luego:

```c
        // Organizacion de memoria fisica segun la parte 2 de la vm:
        // Param Segment -> Const Segment -> Code Segment -> Data Segment -> Extra Segment -> Stack Segment
        pos_const = tam_ps;
        pos_codigo = pos_const + tam_const;
        pos_fin_codigo = pos_codigo + tam_codigo;

        limite_memoria = vmx->memoria.tam_mem_kib * 1024;

        // Valida que el proceso entre en memoria

        if (pos_fin_codigo + tam_datos + tam_extra + tam_pila > limite_memoria) {
            fclose(f);
            logger("[ERROR][CARGA] Memoria insuficiente para alojar proceso (%d bytes requeridos > %d bytes disponibles)\n",
                   pos_fin_codigo + tam_datos + tam_extra + tam_pila, limite_memoria);
            vmx->abortar("Error: Memoria insuficiente.");
        }
```

pos_fin_codigo = Param Segment + Const Segment + Code Segment lo que si estoy considerando toda la memoria.
Si el tamaño requerido exede al limite memoria (que es dinamica) se aborta la inicializacion.

Luego
Cargamos el code segment del archivo a la memoria ram, en el code segment

```c
        i = 0;
        while (i < tam_codigo && fread(&byteLeido, sizeof(uint8_t), 1, f) == 1) {
            vmx->memoria.mem_principal[pos_codigo + i] = byteLeido;
            i++;
        }
        if (i != tam_codigo) {
            fclose(f);
            logger("[ERROR][CARGA] Archivo truncado al leer Code Segment (%d de %d bytes leidos)\n", i, tam_codigo);
            vmx->abortar("Error: El archivo .vmx termino antes de lo esperado o esta danado.");
        }
```

El if da false cuando lei menos bytes que tam_codigo, es decir el archivo vino mal y en este caso debo abortar

Luego

Exactamente lo mismo que antes pero para el const segment.

```c
        // 2. Leer Const Segment y escribir en RAM en pos_const (despues de PS)
        if (tam_const > 0) {
            i = 0;
            while (i < tam_const && fread(&byteLeido, sizeof(uint8_t), 1, f) == 1) {
                vmx->memoria.mem_principal[pos_const + i] = byteLeido;
                i++;
            }
            if (i != tam_const) {
                fclose(f);
                logger("[ERROR][CARGA] Archivo truncado al leer Const Segment (%d de %d bytes leidos)\n", i, tam_const);
                vmx->abortar("Error: El archivo .vmx termino antes de lo esperado o esta danado.");
            }
        }
```

**Tener muy en cuenta que en la cabezera solo viene el code segment y const segment, todo los demas solamente se reservan en memoria, no viene en la cabezera (solo viene el tamaño del mismo)**

Finalmente si todo salio bien cierro el archivo:

```c
        fclose(f);
```