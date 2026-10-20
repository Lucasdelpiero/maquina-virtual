#ifndef PARAM_SEGMENT_H
#define PARAM_SEGMENT_H

#include <stdint.h>
#include "vmx.h"

#define MAX_PARAMS 50
#define MAX_PARAM_LEN 100

// Informacion del Param Segment necesaria para la Subrutina Principal
typedef struct {
    int existe;           // 1 si se creo el segmento (-p presente), 0 si no
    int tamano;           // Tamano total del Param Segment en bytes
    int argc;             // Cantidad total de argumentos (prog.vmx + params)
    uint32_t dir_argv;    // Direccion logica al arreglo argv (0x0000 | offset)
} InfoParamSegment;

// Construye el Param Segment en la memoria fisica de la maquina virtual comenzando en 0x0
// Configura la entrada 0 de la tabla de descriptores y el registro PS
// Retorna la estructura InfoParamSegment con los datos para la subrutina principal
InfoParamSegment construir_param_segment(Vmx *vmx, char nombre_vmx[], char params[][MAX_PARAM_LEN], int nro_params);

#endif // PARAM_SEGMENT_H
