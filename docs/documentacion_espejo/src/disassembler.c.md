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