#include "logger.h"
#include <stdio.h>
#include <stdarg.h>

// Variable interna para controlar si el log esta activo
// Los logs se imprimirán si se paso al iniciar el programa el argumento -debug
static int habilitado = 0;

void logger_habilitar(int nuevo_estado) {
    habilitado = nuevo_estado;
}

/*
    ... Significa Argumentos Variables, lo provee la libreria stdarg.h
    junto con todas las herramientas para usarlos
    va_list args; Declara una variable que contendra los parametros variables
    va_start(args, formato); Inicializa la lista de argumentos
    vprintf(formato, args); Imprime los parametros variables, es un printf que provee la libreria
    va_end(args); Avisa al compilador que terminamos de leer los args variables
*/
void logger(char formato[], ...) {
    va_list args;

    if (!habilitado) {
        return;
    }

    va_start(args, formato);
    vprintf(formato, args);
    va_end(args);
}
