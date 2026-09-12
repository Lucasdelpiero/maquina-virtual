#ifndef INICIALIZADOR_H
#define INICIALIZADOR_H

#include <stdio.h>
#include "memoria.h"
#include "vmx.h"

/*
    Tareas del inicializador:
    -Lee el archivo en binario (.vmx)
    -Valida la version y cabecera
    -Vuelca datos a la memoria principal y tabla de segmentos
    -Escribe en registros las posiciones de CS, DS e IP
*/

// Funciones de carga y configuracion de segmentos
int leer_archivo(char archNom[], Memoria *mem);
void setear_tabla_segmento(Memoria *mem, int tamCS);

// Funciones del subsistema principal Vmx
void inicializar_vmx(Vmx *vmx, int modo_debug, int modo_disassembler);
void cargar_programa(Vmx *vmx, char ruta_archivo[]);

#endif // INICIALIZADOR_H
