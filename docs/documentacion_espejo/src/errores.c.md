```c
static AbortarHandler s_handler = NULL;

void set_abortar_handler(AbortarHandler handler) {
    s_handler = handler;
}
```

AbortarHandler es un puntero a funcion, a la funcion de `abortar()`, la variable estatica junto con la funcion se usa unicamente para el testing.
En ejecucion normal se ejecuta `abortar()` directamente.

```c
void abortar(char mensaje[]) {
    if (s_handler != NULL) {
        s_handler(mensaje);
        return;
    }
    if (mensaje != NULL) {
        // stdeer es el correcto para mostrar errores en c, para stdeer debo usar fprinf, para imprima en el canal de errores
        fprintf(stderr, "%s\n", mensaje);
    }
    // exit cierra el programa, EXIT_FAILURE es una cte que en general vale 1
    exit(EXIT_FAILURE);
}
```

- **`set_abortar_handler`** permite "inyectar" una función alternativa de manejo de errores (patrón de diseño _Hook / Callback_ o _Dependency Injection_ para testing).

- El chequeo `if (s_handler != NULL)` es imprescindible: evita desreferenciar un puntero a función nulo y solo lo ejecuta si fue previamente configurado. Si es `NULL`, continúa con el flujo por defecto: imprimir en `stderr` y llamar a `exit(EXIT_FAILURE)`.

Entonces basicamente, si hay una funcion alternativa de manejo de errores, se usa esa funcion, esta funcion altenativa solo se usara para testing.
Si no la hay, como en la ejecucion normal, se imprime el mensaje usando `fprintf()` porque estamos imprimiendo un error, por eso no usamos printf, debe ir por el canal de errores.

