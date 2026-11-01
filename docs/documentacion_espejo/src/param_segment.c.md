## InfoParamSegment construir_param_segment(

Construye el segmento param segment con los parámetros proporcionados y devuelve información sobre el mismo que sera necesaria posteriormente.

```c
	// Desplazamiento de cada parametro desde el inicio del segmento, dado que los parametros son strings, no todos miden lo mismo
	
	//recordar: offset: cuantos bytes desde el inicio del segmento esta el dato
	// cantidad de parametros + nombre del programa (vmx siempre)
    int offsets[MAX_PARAMS + 1];
    // Offset inicial, logicamente en cero porque no leimos nada aun
    int offset_actual = 0;
    // cantidad de parametros
    int argc;
    int i;
    int j;
    int len;
    // tamaño/desplazamiento total de los parametros, recordar que primero van los strings/parametros e inmediatamente despues el array de punteros a los parametros/strings
    int offset_argv;
    // tamaño total del param segment considerando los parametros y tambien el array de parametros.
    int tamano_total;
    // variable auxilar, un puntero a un elemento del array de punteros a parametros
    uint32_t ptr_argv_elem;
```

### Caso 1: No hay parametros

El usuario no proporciono ningun parametro en la linea comandos, armamos el string con todo vacio logicamente, el puntero al array de parametros quedara en -1 indicando que este no se usara.
Tambien el registro en -1 indica que no se empleara.

```c
    // Caso 1: No hay parametros pasados mediante -p
    if (nro_params <= 0) {
        info.existe = 0;
        info.tamano = 0;
        info.argc = 0;
        info.dir_argv = 0xFFFFFFFF; // Puntero -1 a argv segun especificacion de subrutina principal
        if (vmx != NULL) {
            vmx->registros[PS] = -1; // 0xFFFFFFFF segun especificacion si el segmento no existe
        }
        return info;
    }
```

### Caso 2: Param Segment si definido

```c
    info.existe = 1; // Marco que existe
    argc = 1 + nro_params; // Numero de parametros, se debe incluir tambien el nombre del programa (vmx siempre)
    info.argc = argc;
```


Luego

```c
	// El offset del nombre del programa va en cero. Porque se inicia en cero. El desplazamiento desde el inicio del segmento es cero logicamente.
    offsets[0] = offset_actual;
    
    // Calculamos el offset del 2do elemento (posicion 1)
    len = strlen(nombre_vmx);
    offset_actual = offset_actual + len + 1; // +1 por el '\0'
```

Luego para todos los parametros, calculando su offset y guardandolo en el vector de offsets, como el largo del parametro/string mas el caracter nulo.

Offsets se usara posteriormente para poder armar el array de punteros a parametros, contiene donde empieza cada uno basicamente.

Luego

Tal como se organiza el param segment, primero viene los parametros como string, y después un array de punteros a estos mismos.

```c
    offset_argv = offset_actual;
    tamano_total = offset_argv + (argc * 4);
    info.tamano = tamano_total;
```

Dado que el array comienza donde termina el ultimo parametro, el offset de este es donde termina el ultimo parametro.
El tamaño total del param segment es el tamaño de los parametros/strings o lo que es el mismo el offset que fuimos acumulado sumado a la array de punteros a parámetros, que tendra argc posiciones, cada una un puntero que ocupa 4 bytes.

```c
   info.dir_argv = (uint32_t)(offset_argv & 0xFFFF);
```

En el struct InfoParamSegment se guarda un puntero al array de punteros a parametros.
Dado que este ultimo esta ubicado en alguna parte de la memoria, y dado que no podemos usar punteros a direcciones fisicas, por este motivo este es una direccion logica y por esto tiene en 16 bits altos el segmento y en sus 16 bits bajos el offset.
Siendo en el caso de la parte 2 el param segment el segmento 0.

Luego

```c
    if (vmx != NULL) {
        int limite_memoria = vmx->memoria.tam_mem_kib * 1024;
        if (tamano_total > limite_memoria) {
            logger("[ERROR][PARAM] Param Segment excede la memoria principal (%d bytes).\n", tamano_total);
            if (vmx->abortar != NULL) {
                vmx->abortar("ERROR: Memoria insuficiente para alojar Param Segment.");
            }
            return info;
        }
```

Si el tamaño total del param segment que se sea construir exede a la memoria principal reservada, abortamos.

Luego

Comenzamos a escribir el param segment.

```c
        len = strlen(nombre_vmx);
        for (j = 0; j < len; j++) {
            vmx->memoria.mem_principal[offsets[0] + j] = (uint8_t)nombre_vmx[j];
        }
        // Importante: 0x00 equivale a \0, para armar cadenas validas
        vmx->memoria.mem_principal[offsets[0] + len] = 0x00;
```

Escribir primero el nombre del programa (vmx), por ser string debemos escribir byte a byte a los arreglos (por eso uint8).
Se usa uint8 por ser 8 bits y u para que c no propague signos ni nada.

```c
        // param1 .. paramN
        for (i = 0; i < nro_params; i++) {
            len = strlen(params[i]);
            for (j = 0; j < len; j++) {
                vmx->memoria.mem_principal[offsets[i + 1] + j] = (uint8_t)params[i][j];
            }
            vmx->memoria.mem_principal[offsets[i + 1] + len] = 0x00;
        }
```

Hacemos lo mismo para los parámetros.

Luego:

```c
        // 4. Escribir arreglo argv con los argc punteros de 4 bytes en big-endian
        for (i = 0; i < argc; i++) {
            int pos_puntero = offset_argv + (i * 4);
            ptr_argv_elem = (uint32_t)(offsets[i] & 0xFFFF); // Entrada 0x0000 en 16 bits altos
```

Ahora es cuando usamos el array de offsets creado antes, seguido de los parametros, va el array de punteros a estos parametros.

Un for de 0 a la cantidad de parametos.
Por ser un array, nos movemos de a 4 bytes.

El puntero al elemento es una direccion logica, su desplazamiento sigue siendo la posicion de offsets[] del parametro.

Luego:

```c
            vmx->memoria.mem_principal[pos_puntero]     = (uint8_t)((ptr_argv_elem >> 24) & 0xFF);
            vmx->memoria.mem_principal[pos_puntero + 1] = (uint8_t)((ptr_argv_elem >> 16) & 0xFF);
            vmx->memoria.mem_principal[pos_puntero + 2] = (uint8_t)((ptr_argv_elem >> 8) & 0xFF);
            vmx->memoria.mem_principal[pos_puntero + 3] = (uint8_t)(ptr_argv_elem & 0xFF);
        }
```

El puntero es una direccion logica, y estas son de 32 bits, para guardar esto en el array, hay que hacer esto, cada celda guarda 8 bits, 8 * 4 = 32.
El orden se mantiene, no se invierte para nada.
