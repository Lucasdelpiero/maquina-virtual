
Importante: Demas cosas que se puedan decidir de forma directa y clara mirando la especificacion, y que no requieren decision, por ejemplo si la especificacion ya lo especifica, por favor decide directamente.

## Respondiendo dudas de la ultima vez

#### 1 y 2

"Verificacion empirica del layout del primer bit" + "Version real de la cabezera":

```
PS C:\Users\defin\Desktop\FACULTAD\Facultad 2026-2C\arqui\Maquina-Virtual> .\vmt.exe MOV.asm
[0000] 90 00 0A 0A          |                   MOV        EAX,         10

Instruccion con 2 operandos:

PS C:\Users\defin\Desktop\FACULTAD\Facultad 2026-2C\arqui\Maquina-Virtual> Format-Hex -Path "C:\Users\defin\Desktop\FACULTAD\Facultad 2026-2C\arqui\Maquina-Virtual\MOV.vmx"

   Label: C:\Users\defin\Desktop\FACULTAD\Facultad 2026-2C\arqui\Maquina-Virtual\MOV.vmx

          Offset Bytes                                           Ascii
                 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
          ------ ----------------------------------------------- -----
0000000000000000 56 4D 58 32 36 01 00 04 90 00 0A 0A             VMX26� �� ��
```

Instruccion con 1 y 0 operandos:

```
PS C:\Users\defin\Desktop\FACULTAD\Facultad 2026-2C\arqui\Maquina-Virtual> .\vmt.exe "C:\Users\defin\Desktop\FACULTAD\Facultad 2026-2C\arqui\Maquina-Virtual\Instrucciones\1_operando\jmp_fin.asm"
[0000] 81 00 03             |                   JMP        FIN
[0003] 0F                   | FIN:             STOP
PS C:\Users\defin\Desktop\FACULTAD\Facultad 2026-2C\arqui\Maquina-Virtual> Format-Hex -Path "C:\Users\defin\Desktop\FACULTAD\Facultad 2026-2C\arqui\Maquina-Virtual\Instrucciones\1_operando\jmp_fin.vmx"

   Label: C:\Users\defin\Desktop\FACULTAD\Facultad 2026-2C\arqui\Maquina-Virtual\Instrucciones\1_operando\jmp_fin.vmx

          Offset Bytes                                           Ascii
                 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
          ------ ----------------------------------------------- -----
0000000000000000 56 4D 58 32 36 01 00 04 81 00 03 0F             VMX26� �� ��

PS C:\Users\defin\Desktop\FACULTAD\Facultad 2026-2C\arqui\Maquina-Virtual>
```

#### 3

Verificar que dice la documentacion e intuir como deberia calcularse.

La suma si conlleva riesgo de carry, pero el shift tambien? no era que en el shift simplemente lo que se "iva" se agregaba con ceros y ya?

#### 4

Correcto, validacion a considerar, hay que validar que los dos operandos a validar no den overflow por ser grandes

#### 5

No pide formato exacto, igual esto lo mencionare luego, pero opino que la funcion abortar() recibira un mensaje del tipo:

```
vmx->abortar("Error, No se puede dividir por cero");
```

Por ejemplo, se define el error en la propia funcion, con una linea, abortar() se encarga del printf y process exit
La funcion sera definida en el struct con un puntero a la funcion para simular POO
#### 6

Esto se responde con lo anterior, creo que deberia haber esa funcion que corte en el lugar, para que propagar hacia el ciclo principal? tiene sentido hacer burbujear los errores hacia arriba para la dimension de este proyecto?

#### 7

Esto ya esta definido en el punto 1 y 2 en parte, al leer el archivo, se copia el codigo (que viene luego del tamaño)
El codigo se copia a la memoria principal, a la parte del code segment, esto en el bloque de lectura (nose si es el mismo bloque de inicializacion en si)

El segmento de datos arranca justo despues del codigo, esta en la memoria principal.

#### 8

Los nombres de los archivos para los modulos en C ya de por si deben usar .c y .h y ahi no hay mucho a decidir.
No recuerdo que era Makefile, pero si es para los test, definelos tu.

#### 9

La convencion de nombres es camelCase en general, es la convencion en general para C no?

#### 10

Renzo hara: Desarrollo de operaciones, funciones que retornan el texto para el dissasembler, por ej: funcion add en el modulo de disasembler retorna "ADD"

Algo mas a definir?

#### Otra cosa mas a añadir:

Como se guardan los operandos en memoria?, esto ya existe en la documentacion.
En el registro de OP1, OP2, el primer bit representa el tipo del operando, y los otros 3 el valor.
El tamaño depende del tipo.
O al menos asi lo entendi

## Ideas a tener en cuenta para implementacion

Deberas darme tu opinion.

### Implementar logger

Se usara un logger, para ir añadiendo impresiones a todas las funciones, esto facilitara inmediatamente el debbuging si algo sale mal.
El logger debera mira un flag en el struct vmx que indicara si se lanzo la app en modo dev o no, esto ultimo se da al añadir un argumento mas del tipo `-dev` o algo asi.

El codigo seria algo asi, simulando POO

```c
typedef struct VMX {
    int debug_mode;                          // seteado desde el argv
    void (*logger)(struct VMX *self, const char *msg);
    // resto de campos...
} VMX;

void log_impl(VMX *self, const char *msg) {
    if (self->debug_mode) {
        printf("[LOG] %s\n", msg);
    }
}
```

```c
int main(int argc, char *argv[]) {
    VMX vmx = {0};
    vmx.logger = log_impl;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0) {
            vmx.debug_mode = 1;
        }
    }

    vmx.logger(&vmx, "texto");
    return 0;
}
```

Prefiero no usar macro, acceder como vmx.logger() simplemente.

Por cierto, quiero que el main solo llame a funciones, que no tenga logica especifica como ese for, asi que nose si en inicializar o en otra funcion, leer el binario e inicializar el struct

### Funcion para salir

```
No pide formato exacto, igual esto lo mencionare luego, pero opino que la funcion abortar() recibira un mensaje del tipo:

vmx->abortar("Error, No se puede dividir por cero");

Por ejemplo, se define el error en la propia funcion, con una linea, abortar() se encarga del printf y process exit
La funcion sera definida en el struct con un puntero a la funcion para simular POO
```

Antes ya lo explique bien creo, se usara de forma similar al logger, con la distincion de que no requiere de parametro a la maquina virtual creo.

El abortar tendria su propio printf, no llamaria a logger, no seria necesario, ya que en caso de error se debe loguear siempre.

#### Mas validaciones

Validar que no haya numeros negativos al leer la cabezera? en otras cosas.
No estoy seguro de esto de todos modos.

#### Duda

No me quedo claro porque se recomendo en el plan usar para algunos casos uint para aquellos que no son valores.
Es una buena practica o es requerido usar uint? porque funciona diferente con int32 simple?

#### Aclaracion

La tabla de segmentos no debe inicializarse de memoria, sera sola una funcion para validar el IP.

#### Sobre la inicializacion

Ya se propuso un modulo de inicializacion, pero todo iria alli?
O conviene separar en 2 tipos de inicializaciones

Porque un lado esta la inicializacion intrisica del programa, que no depende del binario a leer, por ej, crear el struct, crear el logger, asignar el logger al struct, etc.

Y por otro lado esta la lectura del binario, y fijacion del ip en 0 (creo) y cosas asi.

Conviene separar para mas claridad o va todo junto esto?, se debe ver como dos cosas separadas o es la misma accion?

#### Respetar diseño del procesador al diseñar el codigo

Debera haber un modulo llamado maquina virtual, que sera adonde estara el struct.
SI lo vemos graficamente:

Dentro de la maquina virtual estaran los modulos:
Inicializador
Decodificador
Registros
Memoria: Se divide en Tabla de segmentos y Memoria Principal
Operador/Operadores (procesa), aqui el array de punteros a funciones

La maquina virtual es que tendra una funcion de nivel bastante alto, que ejecutara el while hasta que el ip sea -1, el stop lo pone asi.

```
while(vmc->registros[IP] != -1) (supongo)
```

## Que sigue ahora

Seguir resolviendo cosas para que quede todo claro

Definir funcioanmiento por encima de todos los modulos, que hacer en el codigo, sin ser especifico.

Los operadores son sencillos y no requieren aclaracion.
Por ej: JNN no requiere aclaracion
Pero JUMP si, es una funcion que tiene logica especifica, debe fijar el code segment a otro lugar creo, asi dentro del while de antes se pasa a leer a otro lugar.
