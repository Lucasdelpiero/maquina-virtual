#include "operandos.h"
#include "memoria.h"
#include "logger.h"

// Extrae los 8 bits superiores (tipo de operando)
int get_tipo(int32_t operando) {
    return (operando >> 24) & 0xFF;
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

// Calcula la direccion logica de memoria: registro base + desplazamiento con signo
int32_t calcular_direccion_logica_memoria(Vmx *vmx, int32_t dato) {
    int cod_reg;
    uint16_t offset_crudo;
    int32_t offset;
    int32_t dir_base;
    int segmento;
    int offset_base;
    int offset_total;
    int32_t dir_logica;

    /* Dato esta armado asi en este caso, por ser un operando de memoria
     31                24 23                 8 7       5 4         0
    ┌────────────────────┬────────────────────┬─────────┬───────────┐
     │     (sin uso, 0)   │  Offset (16 bits)  │ (ceros) │  Cod Reg  │
     │                    │                    │         │  (5 bits) │
    └────────────────────┴────────────────────┴─────────┴───────────┘
    */
    cod_reg = dato & 0x1F;
    offset_crudo = (dato >> 8) & 0xFFFF;
    offset = extender_signo_16_a_32(offset_crudo);
    dir_base = vmx->registros[cod_reg];

    // Separamos la direccion base en codigo de segmento y desplazamiento base
    segmento = (dir_base >> 16) & 0xFFFF;
    offset_base = dir_base & 0xFFFF;

    // Calculamos el desplazamiento final dentro del segmento
    offset_total = offset_base + offset;

    // Si el desplazamiento total es negativo o excede 16 bits, queda fuera del segmento
    if (offset_total < 0 || offset_total > 0xFFFF) {
        logger("[ERROR][OPERANDO] Desplazamiento fuera de segmento: base=%d, offset=%d (total=%d)\n",
               offset_base, offset, offset_total);
        vmx->abortar("Fallo de segmento: acceso fuera de los limites del segmento.");
        return 0;
    }

    // Armamos la direccion logica: 16 bits altos = segmento, 16 bits bajos = desplazamiento
    dir_logica = (segmento << 16) | (offset_total & 0xFFFF);

    return dir_logica;
}

// Resuelve una direccion de memoria traduciendo la direccion logica a fisica
uint16_t resolver_direccion_memoria(Vmx *vmx, int32_t dato) {
    int32_t dir_logica = calcular_direccion_logica_memoria(vmx, dato);
    return traducir_direccion(&vmx->memoria, dir_logica, 4);
}

// Obtiene el valor numerico de 32 bits del operando
int32_t get_valor(Vmx *vmx, int tipo, int32_t dato) {
    if (tipo == TIPO_REGISTRO) {
        int operando = (dato >> 5) & 0x03;
        int registro = dato & 0x1F;

        if (registro >= 10 && registro <= 15) {
            if (operando == 0)
                return vmx->registros[registro];
            else if (operando == 1) // AL..FL
                return (int8_t)(vmx->registros[registro] & 0x000000FF);
            else if (operando == 2) // AH..FH
                return (int8_t)((vmx->registros[registro] & 0x0000FF00) >> 8);
            else if (operando == 3) // AX..FX
                return (int16_t)(vmx->registros[registro] & 0x0000FFFF);
        } else {
            return vmx->registros[registro];
        }
    }
    if (tipo == TIPO_INMEDIATO) { // En este caso dato contendra una constante de 16 bits con el valor directamente
        return extender_signo_16_a_32(dato & 0xFFFF);
    }
    if (tipo == TIPO_MEMORIA) {
        int32_t dir_logica = calcular_direccion_logica_memoria(vmx, dato);
        int bytes_accedidos = get_cantidad_bytes_accedidos(dato);   // Los 3 bits mas significativos indican la cant de bytes a leer

        return get_valor_memoria(&vmx->memoria, dir_logica, bytes_accedidos);
    }

    logger("[ERROR][OPERANDO] Tipo de operando invalido al leer valor: tipo=%d, dato=0x%06X\n", tipo, dato);
    vmx->abortar("Error: Tipo de operando invalido al leer valor.");
    return 0;
}

// Guarda un valor de 32 bits en el destino (registro o memoria)
void set_valor(Vmx *vmx, int tipo, int32_t dato, int32_t valor) {
    if (tipo == TIPO_REGISTRO) {
        int operando = (dato >> 5) & 0x03;
        int registro = dato & 0x1F;

        if (registro >= 10 && registro <= 15) {
            if (operando == 0)
                vmx->registros[registro] = valor;
            else if (operando == 1) // AL..FL
                vmx->registros[registro] =
                    (vmx->registros[registro] & 0xFFFFFF00) | (valor & 0xFF);
            else if (operando == 2) // AH..FH
                vmx->registros[registro] =
                    (vmx->registros[registro] & 0xFFFF00FF) | ((valor & 0xFF) << 8);
            else if (operando == 3) // AX..FX
                vmx->registros[registro] =
                    (vmx->registros[registro] & 0xFFFF0000) | (valor & 0xFFFF);
        } else {
            vmx->registros[registro] = valor;
        }

        logger("[OPERANDO] Registro r%d <= %d (0x%08X)\n", dato & 0x1F, valor, valor);
    } else if (tipo == TIPO_MEMORIA) {
        int32_t dir_logica = calcular_direccion_logica_memoria(vmx, dato);
        int bytes_accedidos = get_cantidad_bytes_accedidos(dato);   // Los 3 bits mas significativos indican la cant de bytes a leer
        set_valor_memoria(&vmx->memoria, dir_logica, valor, bytes_accedidos);
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
        combinado = (actual & 0x0000FFFF) | ((uint32_t)mitad_nueva << 16);
    } else {
        // LDL: carga los 16 bits bajos preservando los 16 bits altos
        combinado = (actual & 0xFFFF0000) | mitad_nueva;
    }

    logger("[OPERANDO] Combinar mitad (%s): anterior=0x%08X, mitad_nueva=0x%04X => nuevo=0x%08X\n",
           cargar_alta ? "LDH (alta)" : "LDL (baja)", actual, mitad_nueva, combinado);

    //Armado el numero, lo seteamos.
    set_valor(vmx, tipo, dato, combinado);
}

// 3 bits mas significativos del operando de memoria contiene el tamano a leer (bits 7..5)
int get_cantidad_bytes_accedidos(int32_t dato) {
    int modificador_operando = (dato >> 5) & 0x07;

    if (modificador_operando == 1)      // 001: lee un byte (b)
        return 1;
    else if (modificador_operando == 2) // 010: lee un word: 2 bytes (w)
        return 2;
    else if (modificador_operando == 4) // 100: lee un long: 4 bytes (l)
        return 4;

    return 4; // Por defecto 4 bytes
}
