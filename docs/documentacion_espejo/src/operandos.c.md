```c
// Extrae los 8 bits superiores (tipo de operando)
int get_tipo(int32_t operando) {
    return (int)((operando >> 24) & 0xFF);
}

// Extrae los 24 bits inferiores (dato del operando)
int32_t get_dato(int32_t operando) {
    return operando & 0x00FFFFFF;
}
```

Para obtener el tipo y el dato solamente aplicamos una mascara y obtenemos la parte del registro que contiene el tipo o el dato.
Recordar que los registros OP1, OP2 solamente contienen el tipo del operando y el valor del mismo.

Luego:

```c
// Extiende el signo de 16 a 32 bits de forma manual
int32_t extender_signo_16_a_32(uint16_t valor16) {
    // Si el bit 15 es 1, es un numero negativo en complemento a dos:
    // rellenamos los 16 bits altos con 1s (0xFFFF0000)
    if (valor16 & 0x8000) {
        return (int32_t)(valor16 | 0xFFFF0000);
    }
    // Si el bit 15 es 0, es positivo: los 16 bits altos quedan en 0
    return (int32_t)valor16;
}
```

Dado que en los registros guardamos valores usando 16bits, debemos extender esos numeros de 16 bits a 32 bits y al hace esto, para valores negativos, puede que haya problemas.
Lo que hace esto es, si el numero es negativo, extiende a 32 bits rellenando con 1s adelante del numero para conservar el signo, algo que por defecto C no hace.

```c
int32_t calcular_direccion_logica_memoria(Vmx *vmx, int32_t dato) {
    int cod_reg;
    uint16_t offset_crudo;
    int32_t offset;
    int32_t dir_base;

    /* Dato esta armado asi en este caso, por ser un operando de memoria
     31                24 23                 8 7       5 4         0
    ┌────────────────────┬────────────────────┬─────────┬───────────┐
    │     (sin uso, 0)   │  Offset (16 bits)  │ (ceros) │  Cod Reg  │
    │                    │                    │         │  (5 bits) │
    └────────────────────┴────────────────────┴─────────┴───────────┘
    */
    cod_reg = dato & 0x1F;
    offset_crudo = (uint16_t)((dato >> 8) & 0xFFFF);
    offset = extender_signo_16_a_32(offset_crudo);
    dir_base = vmx->registros[cod_reg];

    return dir_base + offset;
}
```

Esta funcion solo se usa en el caso de los operandos de memoria, dado que estos operandos tienen una estructura mas compleja, usamos una funcion especializada en esto.

La direccion logica que devolvemos es igual a la dir_base + offset, siendo el offset el desplazamiento que vino en el codigo assembler, por ej `+4` y la dir base es la direccion del registro que se obtiene mediante `vmx->registros[cod_reg]`

Luego

```c
// Obtiene el valor numerico de 32 bits del operando
int32_t get_valor(Vmx *vmx, int tipo, int32_t dato) {
    if (tipo == TIPO_REGISTRO) { //dato contiene el valor el registro operando (OP1/OP2)
        return vmx->registros[dato & 0x1F]; // Al ser operando de tipo registro, se guarda el indice/codigo del registro en los 5 bits menos significativos, luego de esto accedemos al dato directamente.
    }
    if (tipo == TIPO_INMEDIATO) { // En este caso dato contendra una constante de 16 bits con el valor directamente
        return extender_signo_16_a_32((uint16_t)(dato & 0xFFFF));
    }
    if (tipo == TIPO_MEMORIA) {
        int32_t dir_logica = calcular_direccion_logica_memoria(vmx, dato);
        return get_valor_memoria(&vmx->memoria, dir_logica, 4);
    }
```

Luego para el set valor el proceso es muy similar, pero validamos que no se pueda setear un valor de tipo inmediato, ya que no tiene sentido.
