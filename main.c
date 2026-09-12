#include <stdio.h>
#include <stdlib.h>
#include "src\memoria.h"
#include "src\inicializador.h"




int main()
{
    Memoria memoria;
    int32_t valor = 0;

    leer_archivo("filename.vmx", &memoria); //Inicializador lee, carga a memoria y registros
    set_valor_memoria(&memoria, 0X00010006, 0x59B, 3); // Como son 3 bytes escribo 00 05 9B
    valor = get_valor_memoria(memoria, 0X00010006, 3);
    uint32_t operando = 0x0000000A00; // [10]
    set_valor_operando(&memoria,operando,23,1);
    valor = get_valor_operando(memoria, operando, 1);
    operando = 0x0000000F00; // [15]
    set_valor_operando(&memoria,operando,69,1);
    valor = get_valor_operando(memoria, operando, 1);
    set_valor_memoria(&memoria, 0X0001000A,0x11223344, 4);

    int i;
    for(i = 8; i < 20; i++){
        printf("[%d]: %X\n", i, get_valor_memoria(memoria, 0x00010000 + i, 1));
    }



    return 0;
}
