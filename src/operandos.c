#include "operandos.h"
#include "memoria.h"
#include "logger.h"

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
    int32_t dir_logica;
    uint16_t dir_fisica;

    /* Dato esta armado asi en este caso, por ser un operando de memoria
     31                24 23                 8 7       5 4         0
    ┌────────────────────┬────────────────────┬─────────┬───────────┐
    │     (sin uso, 0)   │  Offset (16 bits)  │ (ceros) │  Cod Reg  │
    │                    │                    │         │  (5 bits) │
    └────────────────────┴────────────────────┴─────────┴───────────┘
    */
    cod_reg = dato & 0x1F; // Los operadores de memoria tienen el codigo registro, que tiene 5 bits, aqui accedemos a el.
    offset_crudo = (uint16_t)((dato >> 8) & 0xFFFF);
    offset = extender_signo_16_a_32(offset_crudo);
    dir_base = vmx->registros[cod_reg]; // Obtenemos la direccion base del registro
    dir_logica = dir_base + offset;

    // Carga LAR, traduce direccion física y actualiza MAR
    vmx->registros[LAR] = dir_logica;
    dir_fisica = traducir_direccion(&vmx->memoria, dir_logica, 4);
    vmx->registros[MAR] = ((uint32_t)4 << 16) | ((uint32_t)dir_fisica & 0xFFFF);

    logger("[OPERANDO] Memoria resuelta: reg=%d offset=%d dir_logica=0x%08X -> dir_fisica=0x%04X | LAR=0x%08X MAR=0x%08X\n",
           cod_reg, offset, dir_logica, dir_fisica, vmx->registros[LAR], vmx->registros[MAR]);

    return dir_fisica;
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
        pos_fisica = resolver_direccion_memoria(vmx, dato); //Obtenemos la memoria fisica desde la dinamica para poder acceder
        vmx->registros[MBR] = leer_memoria(&vmx->memoria, pos_fisica, 4);
        return vmx->registros[MBR];
    }

    logger("[ERROR][OPERANDO] Tipo de operando invalido al leer valor: tipo=%d, dato=0x%06X\n", tipo, dato);
    vmx->abortar("Error: Tipo de operando invalido al leer valor.");
    return 0;
}

// Guarda un valor de 32 bits en el destino (registro o memoria)
void set_valor(Vmx *vmx, int tipo, int32_t dato, int32_t valor) {
    uint16_t pos_fisica;

    if (tipo == TIPO_REGISTRO) {
        vmx->registros[dato & 0x1F] = valor; // Idem a lo explicado antes pero ahora seteamos
        logger("[OPERANDO] Registro r%d <= %d (0x%08X)\n", dato & 0x1F, valor, valor);
    } else if (tipo == TIPO_MEMORIA) {
        pos_fisica = resolver_direccion_memoria(vmx, dato);
        vmx->registros[MBR] = valor;
        escribir_memoria(&vmx->memoria, pos_fisica, 4, valor);
    } else if (tipo == TIPO_INMEDIATO) {
        logger("[ERROR][OPERANDO] Intento de escribir en operando inmediato: dato=0x%06X, valor=%d (0x%08X)\n", dato, valor, valor);
        vmx->abortar("Error: Intento de escribir en operando inmediato.");
    } else {
        logger("[ERROR][OPERANDO] Tipo de operando invalido al escribir valor: tipo=%d, dato=0x%06X, valor=%d\n", tipo, dato, valor);
        vmx->abortar("Error: Tipo de operando invalido al escribir valor.");
    }
}

// Read-modify-write para instrucciones LDH y LDL
void combinar_mitad(Vmx *vmx, int tipo, int32_t dato, uint16_t mitad_nueva, int cargar_alta) {
    int32_t actual;
    int32_t combinado;

    actual = get_valor(vmx, tipo, dato);

    /*
        Lo que hace es crear un numero que contiene el valor viejo y el nuevo
        Dado que set_valor modifica todo el valor del registro (los 32 bits)
        lo que debemos hacer es convinar el numero preexistente con el nuevo.
        Dependiendo si queremos cargar la parte baja o alta, ejecutamos una logica u otra.
    */

    if (cargar_alta) {
        // LDH: carga los 16 bits altos preservando los 16 bits bajos
        combinado = (actual & 0x0000FFFF) | ((int32_t)mitad_nueva << 16); //movemos lo nuevo a los altos y reesguardamos lo actual en los bajos
    } else {
        // LDL: carga los 16 bits bajos preservando los 16 bits altos
        combinado = (actual & (int32_t)0xFFFF0000) | (uint16_t)mitad_nueva;
    }

    logger("[OPERANDO] Combinar mitad (%s): anterior=0x%08X, mitad_nueva=0x%04X => nuevo=0x%08X\n",
           cargar_alta ? "LDH (alta)" : "LDL (baja)", actual, mitad_nueva, combinado);

    //Armado el numero, lo seteamos.
    set_valor(vmx, tipo, dato, combinado);
}
