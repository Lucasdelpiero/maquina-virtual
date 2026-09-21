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
