```c
typedef struct {
    int existe; // 1 si esta definido       
    int tamano; // tamaño en bytes que ocupa el param segment          
    int argc;   // cantidad de parametros         
    uint32_t dir_argv; // puntero al array de parametros 
} InfoParamSegment;
```

Struct que contiene informacion sobre el param segment, necesario para la ejecucion de la vm si el usuario tiene instrucciones que utilizan los parametros pasados por la linea de comandos.

**Recordar:** Que todos los tamaños de los segmentos estan definidos en la cabecera del archivo, algunos como el stack segment puede crecer pero el maximo permitido esta definido en la cabezera.

