```c
#include <stdarg.h>

// Variable interna para controlar si el log esta activo
// Los logs se imprimirán si se paso al iniciar el programa el argumento -debug o - dev

static int habilitado = 0;

void logger_habilitar(int nuevo_estado) {
    habilitado = nuevo_estado;
}
```

Habilitado es una variable interna que se fija al inicio, en 1 o 0 dependiendo si se inicio el programa en modo dev o no.
Si no pongo static, al estar fuera de una funcion, C lo interpreta como una variable global, que ademas de ser mala practica, podria dar errores de colision de nombres en otros archivos (porque tambien podran acceder a esta variable)
Como es una variable estatica, debe ser fijada mediante logger_habilitar.

Luego

Para el logger usamos la librería `<stdarg.h>`

```c
void logger(char formato[], ...) {
    va_list args;

    if (!habilitado) {
        return;
    }

    va_start(args, formato);
    vprintf(formato, args);
    va_end(args);
}
```

`...` Significa Argumentos Variables, lo provee la libreria `stdarg.h`
junto con todas las herramientas para usarlos.

`va_list args;` Declara una variable que contendra los parametros variables
`va_start(args, formato);` Inicializa la lista de argumentos
`vprintf(formato, args);` Imprime los parametros variables, es un printf que provee la libreria.

`va_end(args);` Avisa al compilador que terminamos de leer los args variables.

Es un flujo provisto por la libreria, no hay que intentar de entender muy especificamente que hace cada cosa.
