## void depurador_manejar_breakpoint(

Maneja la ejecucion de un breakpoint (SYS 0x0F).
Si no hay archivo .vmi configurado, se ignora y no realiza accion.
Si hay archivo .vmi, guarda la imagen actual y espera una accion (g, q, Enter).

```c
void depurador_manejar_breakpoint(Vmx *vmx) {
    char linea[64];
    char opcion;
```

Es requerido usar un string y un char, debido a que cuando el usuario ingresa la opcion y seguido el enter, se guardan 2 cosas, y no una.
Debemos para ello usar un string, y luego obtener la opcion que eligio el usuario en opcion.

```c
    // Marca que esta instruccion ya disparo una pausa de breakpoint
    fue_breakpoint = 1;
```

Cuando pasamos por un breakpoint, marcamos para no hacer que el usuario ingrese por teclado dos veces seguidas incorrectamente.

Luego

```c
    while (1) {
        printf("[BREAKPOINT] Accion [g = continuar | q = salir | Enter = paso a paso]:\n");

        if (fgets(linea, sizeof(linea), stdin) == NULL) {
            printf("\n");
            exit(0);
        }
```

El bucle con while(1) no presenta un problema de rendimiento ya que dentro del bucle hay un fget que pausa completamente la ejecucion del programa hasta que el usuario ingrese algo.

***

## void depurador_verificar_paso_a_paso(

```c
void depurador_verificar_paso_a_paso(Vmx *vmx) {
    // Si la instruccion actual ya pauso por un breakpoint explicito, no se repite la pausa
    if (fue_breakpoint) {
        fue_breakpoint = 0;
        return;
    }

    // Si esta activo el modo paso a paso, pausa tras ejecutar la instruccion
    if (modo_paso_a_paso) {
        depurador_manejar_breakpoint(vmx);
        fue_breakpoint = 0;
    }
}
```

Verifica si la maquina virtual esta en modo paso a paso.
Si esta activo, dispara automáticamente un breakpoint tras ejecutar la instrucción.

Cuando toca el sys 0, luego cuando se llama a depurador_verificar_paso_a_paso(
sucede que ya se ejecuto un breakpoint, y para no hacer al usuario que ingrese otra vez por teclado, esta la variable fue_breakpoint que en este caso sera 1 y no hara que ingresar otra vez)
Este caso solo sucede cuando se lee el sys 0.