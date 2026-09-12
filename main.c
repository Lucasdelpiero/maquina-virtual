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



    return 0;
}
