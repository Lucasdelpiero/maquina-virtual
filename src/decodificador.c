#include "decodificador.h"
#include <stdio.h>
#include <stdint.h>
#include "memoria.h"

int leer_archivo(char archNom[], Memoria *mem){
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
    //fread(direccion, tamaño, cantidad, archivo);

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
    //printf("tam: %d\n", tamCS);

    fread(&byteLeido, sizeof(uint8_t), 1, f);
    //printf("%X\n", byteLeido);   // hexadecimal mayúscula: FF

    // TODO hacer que cada byte leido vaya a parar a la memoria principal
    i = 0; // posicion de memoria fisica donde va a ir a parar el dato
    while(!feof(f)){
        fread(&byteLeido, sizeof(uint8_t), 1, f);
        //printf("%X\n", byteLeido);   // hexadecimal mayúscula: FF
        //agregar_byte(memoria, byteLeido);
    }

    printf("Corrio todo bien che\n");
    fclose(f);
return 0;
}
