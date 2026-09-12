#include "errores.h"
#include <stdio.h>
#include <stdlib.h>

static AbortarHandler s_handler = NULL;

void set_abortar_handler(AbortarHandler handler) {
    s_handler = handler;
}

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
