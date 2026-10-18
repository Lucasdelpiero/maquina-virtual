```c
// Macros auxiliares para consultar los bits del registro CC
#define FLAG_N(cc) (((uint32_t)(cc) >> 31) & 1)
#define FLAG_Z(cc) (((uint32_t)(cc) >> 30) & 1)
#define FLAG_C(cc) (((uint32_t)(cc) >> 29) & 1)
#define FLAG_V(cc) (((uint32_t)(cc) >> 28) & 1)
```

Son flags booleanos, devuelve si esta activo un flag o no.

```c
    // Bit 31 (N): 1 si el resultado con signo es negativo
    n = ((uint32_t)resultado >> 31) & 1;

    // Bit 30 (Z): 1 si el resultado es igual a cero
    z = (resultado == 0) ? 1 : 0;

    // Bit 29 (C): 1 si se produjo acarreo sin signo
    c = carry ? 1 : 0;

    // Bit 28 (V): 1 si se produjo desbordamiento con signo
    v = overflow ? 1 : 0;

    // Empaqueta los 4 flags en los 4 bits mas significativos; bits 0..27 quedan en cero
    vmx->registros[CC] = (int32_t)((n << 31) | (z << 30) | (c << 29) | (v << 28));
}
```

```c
// Realiza un salto sumando el offset al segmento de codigo (CS)
static void ejecutar_salto(Vmx *vmx, int32_t offset) {
    int tam_codigo = (int)(vmx->memoria.tabla_segmentos[0] & 0xFFFF);
    vmx->registros[IP] = vmx->registros[CS] + offset;
    logger("[EXEC] Salto ejecutado -> IP=0x%08X (offset=%d)\n", vmx->registros[IP], offset);
    if (offset < 0 || offset >= tam_codigo) {
        logger("[WARN][EXEC] Destino de salto fuera del segmento de codigo: offset=%d (tam_codigo=%d). El ciclo se detendra.\n",
               offset, tam_codigo);
    }
}
```

Realizar un salto nomas es actualizar el ip como CS + Offset. Siendo Offset la direccion de memoria adonde se quiere saltar.

Las demas funciones de saltos solo validan que la condicion se cumpla.

```c
void op_jp(Vmx *vmx) {
    int tipo_a = get_tipo(vmx->registros[OP1]);
    int32_t dato_a = get_dato(vmx->registros[OP1]);
    int32_t destino = get_valor(vmx, tipo_a, dato_a);
    int32_t cc = vmx->registros[CC];
    int salta = (FLAG_N(cc) == 0 && FLAG_Z(cc) == 0);

    logger("[EXEC] JP: Salto si positivo (>0) [N=%d Z=%d] => %s\n",
           FLAG_N(cc), FLAG_Z(cc), salta ? "SALTA" : "NO SALTA");
    if (salta) {
        ejecutar_salto(vmx, destino);
    }
}
```

El rotulo se convierte en una constante al pasar por el traductor, esa constante aqui es un operando de tipo inmediato, solo contiene un valor que indica adonde saltar.
`get_valor` devuelve el valor devuelve el valor simplemente, en el caso del operador inmediato solo se hace una extension de signo a 32 bits, que es lo que solemos trabajar, originalmente el dato viene de 16 bits porque asi se guarda en el operando y hay que pasarlo a 32 bits.

Operando tipico:

```c
void op_add(Vmx *vmx) {
    int tipo_a = get_tipo(vmx->registros[OP1]);
    int32_t dato_a = get_dato(vmx->registros[OP1]);
    int tipo_b = get_tipo(vmx->registros[OP2]);
    int32_t dato_b = get_dato(vmx->registros[OP2]);

    int32_t a = get_valor(vmx, tipo_a, dato_a);
    int32_t b = get_valor(vmx, tipo_b, dato_b);
    int32_t res = a + b;

    uint32_t op_a = a;
    uint32_t op_b = b;
    uint64_t suma_sin_signo = (uint64_t)op_a + op_b;
    int carry = (suma_sin_signo > UINT32_MAX);

    int64_t suma_con_signo = (int64_t)a + (int64_t)b;
    // Se da un overflow cuando la suma con signo se sale del rango de 32 bits
    int overflow = (suma_con_signo < INT32_MIN || suma_con_signo > INT32_MAX);

    actualizar_cc(vmx, res, carry, overflow);
    set_valor(vmx, tipo_a, dato_a, res);
    log_operacion_2("ADD", a, b, res, vmx->registros[CC]);
}
```

Obtengo el tipo y dato de los operandos (de los registros OP1, OP2)
Luego teniendo el tipo y dato, obtengo el valor de los operandos.

Si la suma sin signo da mayor al maximo de los 32 bits, es porque se dio un carry.

Para saber facilmente si hubo carry o no, se usa castea a 64 bits los operandos y se hace la operacion, ahora si el resultado de 64 bits exede el maximo de los 32 bits, es porque hubo carry.
Aqui se da un carry, es decir el numero no cabe.

El overflow se da cuando el signo cambio pero no tuve que haber cambiado.
Similar al caso de overflow pero ahora si considerando el signo y tambien se queda un negativo muy chico.

- **Carry (C):** Se preocupa exclusivamente por operaciones **SIN SIGNO** (magnitudes puras de $0$ a $4.294.967.295$).
- **Overflow (V):** Se preocupa exclusivamente por operaciones **CON SIGNO** (en complemento a dos, de $-2.147.483.648$ a $+2.147.483.647$).

Combinar mitad carga en ldl y ldh un numero, descomponiendolo en 2.

