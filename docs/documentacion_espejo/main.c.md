```C
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            *modo_disassembler = 1;
        } else if (strcmp(argv[i], "-dev") == 0) {
            *modo_debug = 1;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "ERROR: Opcion no reconocida '%s'\n", argv[i]);
            fprintf(stderr, "Uso: vmx <archivo.vmx> [-d] [-dev]\n");
            return 0;
        } else {
            if (strlen(nombre_arch) > 0) {
                fprintf(stderr, "ERROR: Se especifico mas de un archivo: '%s' y '%s'\n", nombre_arch, argv[i]);
                return 0;
            }
            if (!tiene_extension_vmx(argv[i])) {
                fprintf(stderr, "ERROR: El archivo '%s' debe tener extension .vmx\n", argv[i]);
                return 0;
            }
            strcpy(nombre_arch, argv[i]);
        }
```

El orden de los argumentos es variable, puede venir el nombre del archivo, antes los argumentos, y demas.

Por ello para cada argumento, verificamos cual argumento es, si es un "-" devolvemos que el nombre no existe, y si se encontro el nombre del archivo, devolvemos que se especifico mas de un archivo, tambien si aun no se encontro el nombre del archivo, validamos que tenga extension .vmx y sino abortamos

**La especificacion no aclara que el orden es fijo**, asi que lo mejor es hacerlo variable.

### main

```c
int main(int argc, char *argv[]) {
    char nombre_arch[260] = "";
    int modo_debug = 0;
    int modo_disassembler = 0;
    Vmx vmx;

    if (!parsear_argumentos(argc, argv, nombre_arch, &modo_debug, &modo_disassembler)) {
        return 1;
    }

    inicializar_vmx(&vmx, modo_debug, modo_disassembler);
    logger("[MAIN] Parametros: Archivo='%s', Disassembler=%s, Debug=%s\n",
           nombre_arch, modo_disassembler ? "SI" : "NO", modo_debug ? "SI" : "NO");
    cargar_programa(&vmx, nombre_arch);

    if (modo_disassembler) {
        desensamblar_programa(&vmx);
    }

    ejecutar_vmx(&vmx);
    logger("[MAIN] Proceso finalizado exitosamente.\n");
```

El orden es:

1. Parsear argumentos: 
2. Inicializar la vmx: inicializar structs, arrays en cero, etc.
3. Cargar el programa (leer archivo binario), se carga a la ram todo y luego se deja de usar el archivo.
4. Si el usuario paso el argumento -d, se ejecuta el dissambler y luego de todos modos se ejecuta el programa
5. Por ultimo se ejecuta todo el programa, leyendo de la memoria ram y ejecutando instrucciones

Si algun paso falla, se hace return 1 y se aborta el programa, en todos los casos se imprime antes el motivo de cierre.