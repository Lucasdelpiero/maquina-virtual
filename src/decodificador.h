#ifndef HEADER_222A8304CF3FC179
#define HEADER_222A8304CF3FC179
#endif // header guard
#include <stdio.h>
#include "memoria.h"

/*
    Tareas del decodificador:
    -Lee el archivo en binario
    -Valida la version del .exe
    -Vuelca datos a la memoria principal y tabla de segmentos
    -Escribe en registros las posiciones de CS, DS e IP
*/


int leer_archivo(char archNom[], Memoria * mem);
