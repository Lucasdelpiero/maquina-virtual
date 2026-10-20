#ifndef VMI_H
#define VMI_H

#include "vmx.h"

// Configuracion interna de la ruta del archivo .vmi
void vmi_configurar_archivo(char ruta[]);
int vmi_tiene_archivo(void);
char *vmi_obtener_archivo(void);

// Guarda el estado actual de la maquina virtual en un archivo binario .vmi
// Si ruta_archivo es NULL o vacio, utiliza la ruta configurada internamente
// Retorna 1 en caso de exito, 0 en caso de error
int guardar_imagen_vmi(Vmx *vmx, char ruta_archivo[], int tam_mem_kib);

// Carga el estado completo de la maquina virtual desde un archivo binario .vmi
// Almacena en tam_mem_kib el tamano de memoria leido desde la cabecera
// Retorna 1 en caso de exito, 0 en caso de error
int cargar_imagen_vmi(Vmx *vmx, char ruta_archivo[], int *tam_mem_kib);

#endif // VMI_H
