## mostrar_valor_sys

```c
static void mostrar_valor_sys(int32_t valor, int tamanio, int formato) {
    int i;
    uint32_t numero;
    int mostrado;

    numero = (uint32_t)valor;
    mostrado = 0;

    // Bit 4 (0x10): Binario
    if (formato & 0x10) {
        if (mostrado) {
            printf(" ");
        }
        for (i = (tamanio * 8) - 1; i >= 0; i--) {
            printf("%d", (int)((numero >> i) & 1));
        }
        mostrado = 1;
    }
```

Se usa numero, siendo que este contiene el numero binario asociado al valor, se usa unsigned porque luego al imprimir binarios se ejecuta un corrimiento `numero >> i` y esto puede hacer que C añada 1s a la izquierda, dado que no queremos eso, trabajamos con numeros sin signo para evitar este comportamiento.

Para imprimir un numero binario, aplicamos corrimientos, imprimimos caracter a caracter usando d (decimal) usando un for.

Hexadecimal, octal y decimal es mas simple, directamente imprimos el valor usando printf con la configuracion de salida correspondiente.

```c
        for (i = 0; i < tamanio; i++) {
            char c = (char)((numero >> (8 * i)) & 0xFF);
            printf("%c", c);
        }
```

Para los char hacemos esto para hallar el caracter.

El tamaño (tamanio) es en byte, osea que si tamaño es 1, son 8 bits, que corresponde a un caracter, cosa que esta bien asi.

`            char c = (char)((numero >> (8 * i)) & 0xFF);`

Esta linea extrae un caracter del numero, el numero puede representar varios caracteres, se multiplica por 8 y por i, para ir desplazando el numero a derecha y quedarme con solo con el caracter que toca imprimir, la mascara pone en ceros los bits altos.

## Leer numero binario

```c
    if (formato & 0x01) {
        // Decimal con signo
        scanf("%d", &num);
        return (int32_t)num;
    } else if (formato & 0x08) {
        // Hexadecimal
        scanf("%x", &unum);
        return (int32_t)unum;
    } else if (formato & 0x04) {
        // Octal
        scanf("%o", &unum);
        return (int32_t)unum;
    } else if (formato & 0x02) {
        // Caracter
        scanf(" %c", &c);
        return (int32_t)(uint8_t)c;
    } else if (formato & 0x10) {
```

Para los formatos basicos, simplemente se lee de teclado y se retorna el valor leido.

Para los binarios es mas complejo:

```c
    } else if (formato & 0x10) {
        // Binario
        scanf("%35s", binario);
        unum = 0;
        i = 0;
        while (binario[i] != '\0') {
            unum = (unum << 1);
            if (binario[i] == '1') {
                unum |= 1;
            }
            i++;
        }
        return (int32_t)unum;
    }
```

Leemos como una cadena de caracteres (array), pero armamos el numero dinamicamente con el while, iterando sobre la cadena y haciendo OR 1 en caso de que sea bit 1.
Se va armando el numero aplicando OR en el ultimo bit. del numero.

Se itera hasta el caracter nulo del string.

## Operador sys para read y write

En ambos casos lo primero que hace es obtener el tipo y valor del primer operando, siendo que sys es una operacion que trabaja con solo 1 operando.

Luego se leen los registros, EAX, EDX, ECX, estos contienen la configuracion de la lectura.
EDX es donde se guarda/toma el valor
EAX es el modo, lectura/escritura
ECX contiene la cantidad y el tamaño de cada elemento a leer/escribir

Para la lectura:

Para toda la cantidad de elementos.
Obtenemos la direccion logica donde se va a guardar el elemento actual, DIR base esta dado por la direccion de EDX, y el offset es el tamaño de cada elemento por i (cantidad) actual.
Traducimos la direccion logica a fisica.
Escribimos en memoria usando la direccion fisica.

```c
printf("[%04X]: ", dir_fisica);
```

Estos printf estan porque la especificacion pide imprimir la direccion de cada dato.

Para la escritura:

Para toda la cantidad de elementos.

Obtenemos la direccion logica del elemento a escribir, DIR base esta dado por la direccion de EDX, y el offset es el tamaño de cada elemento por i (cantidad) actual.
Traducimos la direccion logica a fisica.
Obtenemos el valor de memoria usando la direccion fisica traducida, luego imprimimos usando la funcion mostrar_valor, que varia segun el modo de impresion.
