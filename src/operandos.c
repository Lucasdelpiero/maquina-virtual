#include "operandos.h"
#include "memoria.h"

// Extrae los 8 bits superiores (tipo de operando)
int get_tipo(int32_t operando) {
    return (int)((operando >> 24) & 0xFF);
}

// Extrae los 24 bits inferiores (dato del operando)
int32_t get_dato(int32_t operando) {
    return operando & 0x00FFFFFF;
}

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

// Se usa exclusivamente cuando el operando es de memoria, en otros casos se accede directamente al registro.
// Resuelve una direccion de memoria: registro base + desplazamiento con signo
uint16_t resolver_direccion_memoria(Vmx *vmx, int32_t dato) {
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
    cod_reg = dato & 0x1F; //Los operadores de memoria tienen el codigo registro, que tiene 5 bits, aqui accedemos a el.
    offset_crudo = (uint16_t)((dato >> 8) & 0xFFFF);
    offset = extender_signo_16_a_32(offset_crudo);
    dir_base = vmx->registros[cod_reg]; // Obtenemos la direccion base del registro, es un int que almacena una direccion de memoria

    // Traduce a memoria fisica.
    return traducir_direccion(vmx, dir_base + offset, 4); // dir_base + offset al hacer esto, estamos desplazando la
    //  direccion guardada, es como [EDX+4] EDX contiene una direccion y +4 lo corremos,no tenemos ni idea de a que apunta EDX
}

// Obtiene el valor numerico de 32 bits del operando
int32_t get_valor(Vmx *vmx, int tipo, int32_t dato) {
    uint16_t pos_fisica;

    if (tipo == TIPO_REGISTRO) { //dato contiene el valor el registro operando (OP1/OP2)
        return vmx->registros[dato & 0x1F]; // Al ser operando de tipo registro, se guarda el indice/codigo del registro en los 5 bits menos significativos, luego de esto accedemos al dato directamente.
    }
    if (tipo == TIPO_INMEDIATO) { // En este caso dato contendra una constante de 16 bits con el valor directamente
        return extender_signo_16_a_32((uint16_t)(dato & 0xFFFF));
    }
    if (tipo == TIPO_MEMORIA) {
        pos_fisica = resolver_direccion_memoria(vmx, dato);
        return leer_memoria(vmx, pos_fisica, 4);
    }

    vmx->abortar("Error: Tipo de operando invalido al leer valor.");
    return 0;
}

// Guarda un valor de 32 bits en el destino (registro o memoria)
void set_valor(Vmx *vmx, int tipo, int32_t dato, int32_t valor) {
    uint16_t pos_fisica;

    if (tipo == TIPO_REGISTRO) {
        vmx->registros[dato & 0x1F] = valor;
    } else if (tipo == TIPO_MEMORIA) {
        pos_fisica = resolver_direccion_memoria(vmx, dato);
        escribir_memoria(vmx, pos_fisica, 4, valor);
    } else {
        vmx->abortar("Error: Intento de escribir en operando inmediato.");
    }
}

// Read-modify-write para instrucciones LDH y LDL
void combinar_mitad(Vmx *vmx, int tipo, int32_t dato, uint16_t mitad_nueva, int cargar_alta) {
    int32_t actual;
    int32_t combinado;

    actual = get_valor(vmx, tipo, dato);

    if (cargar_alta) {
        // LDH: carga los 16 bits altos preservando los 16 bits bajos
        combinado = (actual & 0x0000FFFF) | ((int32_t)mitad_nueva << 16);
    } else {
        // LDL: carga los 16 bits bajos preservando los 16 bits altos
        combinado = (actual & (int32_t)0xFFFF0000) | (uint16_t)mitad_nueva;
    }

    set_valor(vmx, tipo, dato, combinado);
}
