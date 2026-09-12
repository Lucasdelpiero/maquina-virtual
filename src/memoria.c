#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "memoria.h"

/*
    Tareas del modulo memoria:
        -Valida accesos a memoria (Dentro de segmento correcto y dentro del limite de memoria)
        -Modifica valores de registros LAR, MAR, MBR cuando se usan operandos de memoria
        -Gestiona acceso a memoria principal:
            -Para esto traduce posicion de memoria logica a fisica usando tabla de segmentos
            -Tambien usa la cant de bytes accedidos, mas que nada para el decodificador y el operador MOV
*/

int32_t get_dir_fisica(Memoria mem, int32_t dir_logica){
    int16_t segmento = (dir_logica>>16) & 1;
    int16_t offset = dir_logica & 0X0000FFFF;

    int32_t dir_base = (mem.tabla_segmentos[segmento] & 0XFFFF0000)>>16;
    int32_t dir_fisica = dir_base + offset;

    return dir_fisica;
}

// NECESITA una DIRECION LOGICA, para sacar datos con OPERANDOS usar get_valor_operando
int32_t get_valor_memoria(Memoria mem, int32_t dir_logica, int bytes_accedidos){
    int32_t resultado=0;
    int i = 0;
    int16_t segmento = (dir_logica>>16);
    int16_t offset = dir_logica & 0X0000FFFF;

    int32_t dir_base = (mem.tabla_segmentos[segmento] & 0XFFFF0000) >> 16;
    int32_t tam_segmento = (mem.tabla_segmentos[segmento] & 0X0000FFFF);
    int32_t dir_fisica = dir_base + offset;

    int32_t limite_segmento = dir_base + tam_segmento;
    int32_t limite_acceso = dir_fisica + bytes_accedidos;

    if (dir_base > dir_fisica){
        printf("ERROR: DIR BASE MAYOR QUE DIR FISICA\n");
        exit(1);
    }
    if (limite_segmento < limite_acceso){
        printf("ERROR: LIM SEGMENTO MENOR QUE LIM ACCESO\n");
        exit(1);
    }

    //Concatena el valor segun la cantidad de bytes requeridos;
    resultado = mem.mem_principal[dir_fisica];
    for(i = 1; i < bytes_accedidos; i++){ // Concatenacion con | no funciona aca
        resultado = resultado << 8;
        resultado  += mem.mem_principal[dir_fisica + i];
    }

    return resultado;
}

// Escribe byte a byte
void escribir_byte(Memoria * mem, int pos, uint8_t dato){
    // Validar memoria
    if (pos >= TAM_MEMORIA_PRINCIPAL){
        printf("ERROR: Memoria principal se lleno\n");
        exit(1);
    } else if (pos < 0){
        printf("ERROR: se quiere acceder a posicion negativa\n");
        exit(1);
    }

    // Agrega dato leido a la memoria principal
    mem->mem_principal[pos] = dato;

    //printf("Mem[%d]: %X\n", pos, dato);
}

// NECESITA una direccion LOGICA, para escribir usando operandos usar set_valor_operando
void set_valor_memoria(Memoria *mem, int32_t dir_logica, int32_t dato, int cant_accedidos){
    int32_t dir_fisica = get_dir_fisica(*mem, dir_logica);
    int i;
    int bytes[4];
    // Separa un 32 bits en 4 elementos de 8 bits
    for(i=0; i < 4; i++){
        bytes[3- i] = (dato>> 8 * i ) & 0XFF;
    }
    // Escribe byte a byte en memoria
    int offset = 0;
    int primer_byte = 4 - cant_accedidos;
    for(i = primer_byte; i < 4; i++){
        escribir_byte(mem, dir_fisica + offset , bytes[i]);
        offset++;
    }

}

// Recibe [offset (16 bits) | registro(5 bits)] con el registro siendo un puntero, DS O CS para funcionar bien
void set_valor_operando(Memoria *mem, int32_t operando, int32_t dato, int cant_accedidos){
    uint32_t dir_logica;
    uint32_t offset = operando >> 8;
    uint32_t registro = operando & 0XFF;
    uint32_t tempDS = 0X00010000;
    if (registro == 0) // Si no hay registro se pone el DS por defecto
        registro = tempDS;

    dir_logica = registro + offset;

    set_valor_memoria(mem, dir_logica, dato, cant_accedidos);
}

uint32_t get_valor_operando(Memoria mem, int32_t operando, int cant_accedidos){
    uint32_t dir_logica;
    uint32_t offset = operando >> 8;
    uint32_t registro = operando & 0XFF;
    uint32_t tempDS = 0X00010000;
    if (registro == 0) // Si no hay registro se pone el DS por defecto
        registro = tempDS;

    dir_logica = registro + offset;

    return get_valor_memoria(mem, dir_logica, cant_accedidos);
}

void inicializar_memoria(Memoria *mem){
    int i;
    for(i=0; i < TAM_MEMORIA_PRINCIPAL; i++){
        mem->mem_principal[i] = 0;
    }
}
