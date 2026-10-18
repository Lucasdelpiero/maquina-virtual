
### Mapear nombres

```c
// Tabla de mnemonicos indexada por codigo de operacion (0..31)
static const char *tabla_mnemonicos[32] = {
    [0x00] = "SYS",
    [0x01] = "JMP",
    [0x02] = "JP",
    ...
    [0x1B] = "SHR",
    [0x1C] = "SAR",
    [0x1D] = "LDL",
    [0x1E] = "LDH",
    [0x1F] = "RND"
};

// Tabla de nombres de registros indexada por codigo (0..31)
static const char *tabla_registros[32] = {
    [0]  = "IP",
    [1]  = "OPC",
    [2]  = "OP1",
    [3]  = "OP2",
    [4]  = "LAR",
    [5]  = "MAR",
    ...
```

Primero que nada mapeamos los codigos de operacion a strings, para poder obtener el memotecnico equivalente rapidamente.
Idem para los registros.

### Funciones para obtener nombre

```c
const char* obtener_mnemonico(int opc) {
    if (opc >= 0 && opc < 32 && tabla_mnemonicos[opc] != NULL) {
        return tabla_mnemonicos[opc];
    }
    return "???";
}

const char* obtener_nombre_registro(int cod_reg) {
    if (cod_reg >= 0 && cod_reg < 32 && tabla_registros[cod_reg] != NULL) {
        return tabla_registros[cod_reg];
    }
    return "???";
}
```

Funciones helper para obtener el nombre del registro o memotecnico asociado, sino coincide devuelve ??? y no corta directamente.

### formatear operando

```c
void formatear_operando(char destino[], int tipo, int32_t dato) {
    int cod_reg;
    int32_t offset;
    int32_t valor_inmediato;

    destino[0] = '\0';

    if (tipo == TIPO_REGISTRO) {
        cod_reg = (int)(dato & 0x1F);
        sprintf(destino, "%s", obtener_nombre_registro(cod_reg));
    } else if (tipo == TIPO_INMEDIATO) {
        valor_inmediato = extender_signo_16_a_32((uint16_t)(dato & 0xFFFF));
        sprintf(destino, "%d", valor_inmediato);
    } else if (tipo == TIPO_MEMORIA) {
        cod_reg = (int)(dato & 0x1F);
        offset = extender_signo_16_a_32((uint16_t)((dato >> 8) & 0xFFFF));
        if (offset == 0) {
            sprintf(destino, "[%s]", obtener_nombre_registro(cod_reg));
        } else if (offset > 0) {
            sprintf(destino, "[%s+%d]", obtener_nombre_registro(cod_reg), offset);
        } else {
            sprintf(destino, "[%s%d]", obtener_nombre_registro(cod_reg), offset);
        }
    }
}
```

destino es el string que armamos dinamicamente que vamos a imprimir en consola.
Lo inicializamos en \0 porque para que ese array de char sea realmente un string.

`sprintf` es para armar el string, en una sola linea, primero va la variable, luego el formato, y luego los n argumentos.

Si es operando de registro, asignamos el nombre directamente
Si es inmediato, obtenemos el numero y asignamos el numero
Si es de memoria es mas complejo, dependiendo si tiene offset o no, imprimimos +, - , o directamente el nombre del registro entre corchetes.

### Desensamblar programa

Esta es la funcion de entrada para ejecutar el disassembler.

```c
    int dir = 0;
    int tam_codigo = vmx->memoria.tabla_segmentos[0] & 0xFFFF;
    char buffer[128];
```

Lo primero que hace es obtener el tamaño del codigo, esto lo obtiene desde la tabla de segmentos en la posicion 0, el codesegment corresponde al segmento 0. Aplicamos la mascara porque solo nos interesa el tamaño, no nos interesa la direccion fisica equivalente.

buffer es el string donde vamos a almacenar toda una instruccion en assembler, hasta 128 caracteres.

```c
    while (dir < tam_codigo) {
        int cant_bytes = desensamblar_en_direccion(vmx, dir, buffer, sizeof(buffer));
        if (cant_bytes <= 0) break;
        printf("%s\n", buffer);
        dir += cant_bytes;
    }
```

Mientras no termine de leer todo el code segment, llamo a desemsablar en direccion, esta funcion es la 2da mas importante, que literalmente lee una instruccion del code segment y guarda en el buffer, poniendo como string vacio el buffer previamente.

La misma funcion devuelve cual fue el tamaño de la instruccion, recordar que el tamaño de las instrucciones es variable.

La validacion `cant_bytes <= 0` sirve para evitar un posible bucle infinito, porque en desensamblar en direccion esta la validacion `dir_fisica >= TAM_MEMORIA_PRINCIPAL`, si esta validacion falla, la funcion devuelve cero. Y luego sumariamos a dir cero, el while no terminaria nunca.

Luego con el printf() imprimimos el buffer generamos.

Y avanzamos la direccion siguiente a leer sumando la cantidad de bytes, esto hara que dir apunte a la siguiente instruccion a desensamblar.

### desemsablar en direccion

Esta función lee una instrucción del code segment, apuntada por el parámetro `dir` y guarda la instrucción decodificada (ya en assembler) en el string buffer que se le pasa como parámetro.

```c
    if (dir_fisica < 0 || dir_fisica >= TAM_MEMORIA_PRINCIPAL) {
        if (tam_buffer > 0) buffer[0] = '\0';
        return 0;
    }
```

Primero validamos que la direccion dir pasada sea valida, que no sea cero negativa o afuera de la memoria.
Ponemos el buffer como string nulo en caso de que algo salga mal, para evitar imprimir cosas en un caso raro.

```c
    uint8_t primer_byte = vmx->memoria.mem_principal[dir_fisica];
```

Obtenemos el primer byte de la direccion dir del parametro, este es fijo, contiene el tipos de los operandos y el codigo de operacion, siempre mide 1 byte.

```c
    if (primer_byte == OP_STOP) {
        opc = OP_STOP;
    } else if (((primer_byte >> 4) & 0x03) == 0) {
        opc = primer_byte & 0x1F;
        tipo_a = (primer_byte >> 6) & 0x03;

        if (tipo_a == TIPO_REGISTRO) {
            dato_a = vmx->memoria.mem_principal[pos] & 0x1F;
            pos += 1;
        } else if (tipo_a == TIPO_INMEDIATO) {
            uint16_t val16 = ((uint16_t)vmx->memoria.mem_principal[pos] << 8) | (uint16_t)vmx->memoria.mem_principal[pos + 1];
            dato_a = val16;
            pos += 2;
        } else if (tipo_a == TIPO_MEMORIA) {
            uint16_t offset = ((uint16_t)vmx->memoria.mem_principal[pos] << 8) | (uint16_t)vmx->memoria.mem_principal[pos + 1];
            uint8_t cod_reg = vmx->memoria.mem_principal[pos + 2] & 0x1F;
            dato_a = ((int32_t)offset << 8) | cod_reg;
            pos += 3;
        }
```

Si el codigo de operacion es el del stop, solo tenemos que asignar al opc la constante OP_STOP que contendra el string correspondiente, es decir 00001111

Si el tipo de codigo de operacion corresponde a operaciones de un solo operando, obtenemos codigo de operacion en opc y el tipo del operando a solamente, ya que no habra operando b.
Cuando la instruccion es de un operando, los bits 5 y 4 son cero, porque las operaciones de un operando van de 0 a 10, es decir de 0000 a 01010.
El bit 4 queda en cero y el bit 5 queda en cero por especificacion.

En las instrucciones de dos operandos, el bit 5 y 4 guardan el tipo operando A, de modo que estos nunca podran ser cero.
Y el codigo de operacion se guarda de los bits 0 a 3, es decir 4 bits, hasta 16 combinaciones.

Luego de obtener el tipo, dependiendo si es de memoria, inmediato, o de registro, se aplica diferente logica.
Primero pos es dir + 1, porque el primer byte siempre es el mismo, luego vienen los operandos.

Si es de registro, debo obtener el registro, en `vmx->memoria.mem_principal[pos] & 0x1F;` obtengo el codigo del registro en el codesegment, dado que un registro ocupa un byte, luego a pos le sumo un solo bye.
Aplico la mascara 1F para obtener el codigo, que esta en los ultimos 5 bits, por las dudas de que haya sido rellenado con 1s adelante.

Para los inmediatos, debemos leer 2 bytes, ya que el valor se guarda en dos bytes, primero corremos 8 bits para agarrar los 8 bits altos de la memoria, y luego se concatena los 8 bits bajos q se guardan en pos+1
Avanzamos 2 en pos, ya que leimos dos bytes.

Para el tipo memoria desplazamos 3, el proceso es igual al anterior, leemos los 2 primeros bytes que representan el offset (por ej el +10 de DS+10) y en el proximo o 3er byte se guarda el codigo del registro (Aqui aplicamos mascara y nos quedamos con sus ultimos 5 bits)
Avanzamos pos 3 bytes.

Luego

```c
   } else {
        opc = 0x10 | (primer_byte & 0x0F);
        tipo_b = (primer_byte >> 6) & 0x03;
        tipo_a = (primer_byte >> 4) & 0x03;

        // B se codifica primero en el binario
        if (tipo_b == TIPO_REGISTRO) {
            dato_b = vmx->memoria.mem_principal[pos] & 0x1F;
            pos += 1;
        } else if (tipo_b == TIPO_INMEDIATO) {
            uint16_t val16 = ((uint16_t)vmx->memoria.mem_principal[pos] << 8) | (uint16_t)vmx->memoria.mem_principal[pos + 1];
            dato_b = val16;
            pos += 2;
        } else if (tipo_b == TIPO_MEMORIA) {
            uint16_t offset = ((uint16_t)vmx->memoria.mem_principal[pos] << 8) | (uint16_t)vmx->memoria.mem_principal[pos + 1];
            uint8_t cod_reg = vmx->memoria.mem_principal[pos + 2] & 0x1F;
            dato_b = ((int32_t)offset << 8) | cod_reg;
            pos += 3;
        }

        // A se codifica segundo
        if (tipo_a == TIPO_REGISTRO) {
            dato_a = vmx->memoria.mem_principal[pos] & 0x1F;
            pos += 1;
        } else if (tipo_a == TIPO_INMEDIATO) {
            uint16_t val16 = ((uint16_t)vmx->memoria.mem_principal[pos] << 8) | (uint16_t)vmx->memoria.mem_principal[pos + 1];
            dato_a = val16;
            pos += 2;
        } else if (tipo_a == TIPO_MEMORIA) {
            uint16_t offset = ((uint16_t)vmx->memoria.mem_principal[pos] << 8) | (uint16_t)vmx->memoria.mem_principal[pos + 1];
            uint8_t cod_reg = vmx->memoria.mem_principal[pos + 2] & 0x1F;
            dato_a = ((int32_t)offset << 8) | cod_reg;
            pos += 3;
        }
    }
```

Si el codigo de operacion es de dos operandos, el proceso es el mismo, es codigo repetido, pero hace lo mismo para un operando A y para el otro B.

Al desemsamblar, el operando A va primero, por como funciona nuestro assembler, pero al leer de memoria, primero viene el operando B y luego el A, de aqui porque termina como alrevez.

Luego

```c
    int cant_bytes = pos - dir_fisica;
    const char *mnem = obtener_mnemonico(opc);
```

Obtenemos el memotecnico, usando el opc hallado, y la cantidad de bytes a desplazarnos para la proxima lectura (es el cant_bytes que se retorna)

```c
    formatear_operando(op_a_str, tipo_a, dato_a);
    formatear_operando(op_b_str, tipo_b, dato_b);
```

Seguido de los operandos, obtenemos los string equivalente a los operandos.
Si B o A es tipo ninguno, no se nada.

```c
    char bytes_hex[32] = "";
    char byte_str[8];
    int i;
    for (i = 0; i < cant_bytes && (dir_fisica + i) < TAM_MEMORIA_PRINCIPAL; i++) {
        if (i > 0) {
            strcat(bytes_hex, " "); //dejar espacio entre palabras
        }
        sprintf(byte_str, "%02X", vmx->memoria.mem_principal[dir_fisica + i]);
        strcat(bytes_hex, byte_str);
    }

    if (opc == OP_STOP) {
        snprintf(buffer, tam_buffer, "[%04X] %-17s | %s", dir_fisica, bytes_hex, mnem);
    } else if (opc <= 0x0A) {
        snprintf(buffer, tam_buffer, "[%04X] %-17s | %s %s", dir_fisica, bytes_hex, mnem, op_a_str);
    } else {
        snprintf(buffer, tam_buffer, "[%04X] %-17s | %s %s, %s", dir_fisica, bytes_hex, mnem, op_a_str, op_b_str);
    }

    return cant_bytes;
```

Primero armamos la instruccion en hexa que tambien debemos imprimir, es lo de: "[B1 00 0A...]"

Luego armamos los string (el codigo assembler desemsamblado junto la instruccion al principio) y retornamos la cant de bytes avanzados.
Varia segun si es codigo de operacion de 0,1,2 operandos.