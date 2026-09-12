#include <stdio.h>
#include <stdint.h>
#include "memoria.h"
#include "inicializador.h"

/*
    Tareas del decodificador:
    -Lee el archivo en binario
    -Valida la version del .exe
    -Vuelca datos a la memoria principal y tabla de segmentos
    -Escribe en registros las posiciones de CS, DS e IP
*/

// TODO falta que ponga los registros con el IP, CS Y DS
int leer_archivo(char archNom[], Memoria *mem){
    inicializar_memoria(mem);
    uint8_t byteLeido;
    char identificador[5];
    int version;
    int tamCS = 0;
    int temp[2];
    int i;

    FILE *f = fopen(archNom, "rb");
    if (f == NULL) {
        printf("Error al abrir el archivo\n");
    }

    // Carga version y tamaño
    for(i = 0; i < 5; i++){
        fread(&byteLeido, sizeof(uint8_t), 1, f);
        //printf("%c", byteLeido);
        identificador[i] = byteLeido;
    }
    //printf("\n");

    fread(&byteLeido, sizeof(uint8_t), 1, f);
    version = byteLeido;
    //printf("version: %d\n", version);

    for(i=0; i < 2; i++){
        fread(&byteLeido, sizeof(uint8_t), 1, f);
        temp[i] = byteLeido;
        //printf("byte [%d]: %X\n", i, byteLeido);

    }
    tamCS = temp[0] | temp[1];

    // TODO hacer que cada byte leido vaya a parar a la memoria principal
    i = 0; // posicion de memoria fisica donde va a ir a parar el dato
    fread(&byteLeido, sizeof(uint8_t), 1, f);
    while(!feof(f)){
        //printf("[%d]: %X\n", i, byteLeido);   // hexadecimal mayúscula: FF
        escribir_byte(mem, i,byteLeido);
        fread(&byteLeido, sizeof(uint8_t), 1, f);
        i++;
    }
    fclose(f);
    setear_tabla_segmento(mem, tamCS);
    return 0;
}


// Toma el valor de tamaño de codigo dado en el primer byte y la constante de tamaño de MEM PRINCIPAL y lo usa para calcular la tabla de segmentos
void setear_tabla_segmento(Memoria *mem, int tamCS){
    int16_t inicio_cs = 0; // Puede llegar a ser util
    int16_t fin_cs = tamCS;
    int16_t inicio_ds = fin_cs;
    int16_t fin_ds = TAM_MEMORIA_PRINCIPAL - fin_cs;
    mem->tabla_segmentos[0] = (inicio_cs<<16) + fin_cs;
    mem->tabla_segmentos[1] = (inicio_ds<<16) +  fin_ds;

    int i;
    for(i=2; i < TAM_SEGMENTOS; i++){
        mem->tabla_segmentos[i] = 0xFFFFFFFF;
    }

}
