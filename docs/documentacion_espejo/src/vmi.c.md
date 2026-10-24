### void vmi_configurar_archivo(

```c
static char ruta_vmi_actual[260] = "";

void vmi_configurar_archivo(char ruta[]) {
    if (ruta != NULL) {
        strcpy(ruta_vmi_actual, ruta);
    } else {
        ruta_vmi_actual[0] = '\0';
    }
}
```

Setea la ruta del archivo vmi

El tamaño de la memoria se guarda en 16 bits o 2 bytes.

```c
// Escribe 2 bytes en formato big-endian
static void escribir_uint16_be(FILE *f, int valor) {
    uint8_t buffer[2];
    buffer[0] = (uint8_t)((valor >> 8) & 0xFF);
    buffer[1] = (uint8_t)(valor & 0xFF);
    fwrite(buffer, sizeof(uint8_t), 2, f);
}

// Lee 2 bytes en formato big-endian
static int leer_uint16_be(FILE *f, int *resultado) {
    uint8_t buffer[2];
    if (fread(buffer, sizeof(uint8_t), 2, f) != 2) {
        return 0;
    }
    *resultado = ((int)buffer[0] << 8) | (int)buffer[1];
    return 1;
}
```

### int guardar_imagen_vmi(

Guarda el estado actual de la maquina virtual en un archivo binario .vmi
Si ruta_archivo es NULL o vacio, utiliza la ruta configurada internamente

```c
    FILE *f;
    uint8_t version;
    int i;
    // Cantidad de bytes de la memoria principal actual que se debe volcar al vmi
    int bytes_memoria;
    // puntero a la ruta del vmi
    char *ruta_final;
```

### int cargar_imagen_vmi

Carga el estado completo de la maquina virtual desde un archivo binario .vmi
Almacena en tam_mem_kib el tamano de memoria leido desde la cabecera

```c
    FILE *f;
    // VMI2026 o el que sea
    char identificador[6];
    uint8_t version;
    // cantidad de memoria ram que requiere cargar la imagen (la que se uso en la ejecucion anterior), la memoria de lla maquina virtual debera ser esta
    int mem_kib;
    // Cantidad de bytes a volcar ahora a la ram (vmi -> ram)
    int bytes_memoria;
    int i;
    // variable auxiliar para cargar registros
    uint32_t valor32;
```