#include "operadores.h"
#include "operandos.h"
#include "sys.h"
#include "logger.h"
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

// Macros auxiliares para consultar los bits del registro CC
#define FLAG_N(cc) (((uint32_t)(cc) >> 31) & 1)
#define FLAG_Z(cc) (((uint32_t)(cc) >> 30) & 1)
#define FLAG_C(cc) (((uint32_t)(cc) >> 29) & 1)
#define FLAG_V(cc) (((uint32_t)(cc) >> 28) & 1)

// Actualiza los bits N, Z, C, V del registro CC
void actualizar_cc(Vmx *vmx, int32_t resultado, int carry, int overflow) {
    uint32_t n;
    uint32_t z;
    uint32_t c;
    uint32_t v;

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

// Macro auxiliar para imprimir el log de una operacion aritmetica o logica
static void log_operacion_2(char nombre[], int32_t a, int32_t b, int32_t res, int32_t cc) {
    logger("[EXEC] %s: opA=%d (0x%08X), opB=%d (0x%08X) => res=%d (0x%08X) [N=%d Z=%d C=%d V=%d]\n",
           nombre, a, a, b, b, res, res,
           FLAG_N(cc), FLAG_Z(cc), FLAG_C(cc), FLAG_V(cc));
}

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

// --- Instrucciones de control y saltos ---

void op_stop(Vmx *vmx) {
    logger("[EXEC] STOP: Deteniendo la ejecucion de la maquina virtual (IP = -1)\n");
    vmx->registros[IP] = -1;
}

void op_jmp(Vmx *vmx) {
    int tipo_a = get_tipo(vmx->registros[OP1]);
    int32_t dato_a = get_dato(vmx->registros[OP1]);
    int32_t destino = get_valor(vmx, tipo_a, dato_a);

    logger("[EXEC] JMP: Salto incondicional hacia offset=%d (CS=0x%08X)\n", destino, vmx->registros[CS]);
    ejecutar_salto(vmx, destino);
}

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

void op_jn(Vmx *vmx) {
    int tipo_a = get_tipo(vmx->registros[OP1]);
    int32_t dato_a = get_dato(vmx->registros[OP1]);
    int32_t destino = get_valor(vmx, tipo_a, dato_a);
    int32_t cc = vmx->registros[CC];
    int salta = (FLAG_N(cc) == 1 && FLAG_Z(cc) == 0);

    logger("[EXEC] JN: Salto si negativo (<0) [N=%d Z=%d] => %s\n",
           FLAG_N(cc), FLAG_Z(cc), salta ? "SALTA" : "NO SALTA");
    if (salta) {
        ejecutar_salto(vmx, destino);
    }
}

void op_jz(Vmx *vmx) {
    int tipo_a = get_tipo(vmx->registros[OP1]);
    int32_t dato_a = get_dato(vmx->registros[OP1]);
    int32_t destino = get_valor(vmx, tipo_a, dato_a);
    int32_t cc = vmx->registros[CC];
    int salta = (FLAG_Z(cc) == 1);

    logger("[EXEC] JZ: Salto si cero (==0) [Z=%d] => %s\n",
           FLAG_Z(cc), salta ? "SALTA" : "NO SALTA");
    if (salta) {
        ejecutar_salto(vmx, destino);
    }
}

void op_jc(Vmx *vmx) {
    int tipo_a = get_tipo(vmx->registros[OP1]);
    int32_t dato_a = get_dato(vmx->registros[OP1]);
    int32_t destino = get_valor(vmx, tipo_a, dato_a);
    int32_t cc = vmx->registros[CC];
    int salta = (FLAG_C(cc) == 1);

    logger("[EXEC] JC: Salto si carry (C=1) [C=%d] => %s\n",
           FLAG_C(cc), salta ? "SALTA" : "NO SALTA");
    if (salta) {
        ejecutar_salto(vmx, destino);
    }
}

void op_jv(Vmx *vmx) {
    int tipo_a = get_tipo(vmx->registros[OP1]);
    int32_t dato_a = get_dato(vmx->registros[OP1]);
    int32_t destino = get_valor(vmx, tipo_a, dato_a);
    int32_t cc = vmx->registros[CC];
    int salta = (FLAG_V(cc) == 1);

    logger("[EXEC] JV: Salto si overflow (V=1) [V=%d] => %s\n",
           FLAG_V(cc), salta ? "SALTA" : "NO SALTA");
    if (salta) {
        ejecutar_salto(vmx, destino);
    }
}

void op_jnp(Vmx *vmx) {
    int tipo_a = get_tipo(vmx->registros[OP1]);
    int32_t dato_a = get_dato(vmx->registros[OP1]);
    int32_t destino = get_valor(vmx, tipo_a, dato_a);
    int32_t cc = vmx->registros[CC];
    int salta = (FLAG_N(cc) == 1 || FLAG_Z(cc) == 1);

    logger("[EXEC] JNP: Salto si no positivo (<=0) [N=%d Z=%d] => %s\n",
           FLAG_N(cc), FLAG_Z(cc), salta ? "SALTA" : "NO SALTA");
    if (salta) {
        ejecutar_salto(vmx, destino);
    }
}

void op_jnn(Vmx *vmx) {
    int tipo_a = get_tipo(vmx->registros[OP1]);
    int32_t dato_a = get_dato(vmx->registros[OP1]);
    int32_t destino = get_valor(vmx, tipo_a, dato_a);
    int32_t cc = vmx->registros[CC];
    int salta = (FLAG_N(cc) == 0);

    logger("[EXEC] JNN: Salto si no negativo (>=0) [N=%d] => %s\n",
           FLAG_N(cc), salta ? "SALTA" : "NO SALTA");
    if (salta) {
        ejecutar_salto(vmx, destino);
    }
}

void op_jnz(Vmx *vmx) {
    int tipo_a = get_tipo(vmx->registros[OP1]);
    int32_t dato_a = get_dato(vmx->registros[OP1]);
    int32_t destino = get_valor(vmx, tipo_a, dato_a);
    int32_t cc = vmx->registros[CC];
    int salta = (FLAG_Z(cc) == 0);

    logger("[EXEC] JNZ: Salto si no cero (!=0) [Z=%d] => %s\n",
           FLAG_Z(cc), salta ? "SALTA" : "NO SALTA");
    if (salta) {
        ejecutar_salto(vmx, destino);
    }
}

void op_not(Vmx *vmx) {
    int tipo_a = get_tipo(vmx->registros[OP1]);
    int32_t dato_a = get_dato(vmx->registros[OP1]);
    int32_t a = get_valor(vmx, tipo_a, dato_a);
    int32_t res = ~a;

    actualizar_cc(vmx, res, 0, 0);
    set_valor(vmx, tipo_a, dato_a, res);
    logger("[EXEC] NOT: opA=%d (0x%08X) => res=%d (0x%08X) [N=%d Z=%d C=0 V=0]\n",
           a, a, res, res, FLAG_N(vmx->registros[CC]), FLAG_Z(vmx->registros[CC]));
}

// --- Instrucciones de dos operandos ---

void op_mov(Vmx *vmx) {
    int tipo_a = get_tipo(vmx->registros[OP1]);
    int32_t dato_a = get_dato(vmx->registros[OP1]);
    int tipo_b = get_tipo(vmx->registros[OP2]);
    int32_t dato_b = get_dato(vmx->registros[OP2]);
    int32_t b = get_valor(vmx, tipo_b, dato_b);

    actualizar_cc(vmx, b, 0, 0);
    set_valor(vmx, tipo_a, dato_a, b);
    logger("[EXEC] MOV: cargando valor=%d (0x%08X) en destino [N=%d Z=%d C=0 V=0]\n",
           b, b, FLAG_N(vmx->registros[CC]), FLAG_Z(vmx->registros[CC]));
}

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

void op_sub(Vmx *vmx) {
    int tipo_a = get_tipo(vmx->registros[OP1]);
    int32_t dato_a = get_dato(vmx->registros[OP1]);
    int tipo_b = get_tipo(vmx->registros[OP2]);
    int32_t dato_b = get_dato(vmx->registros[OP2]);

    int32_t a = get_valor(vmx, tipo_a, dato_a);
    int32_t b = get_valor(vmx, tipo_b, dato_b);
    int32_t res = a - b;

    int carry = (a < b);

    int64_t resta_con_signo = (int64_t)a - (int64_t)b;
    int overflow = (resta_con_signo < INT32_MIN || resta_con_signo > INT32_MAX);

    actualizar_cc(vmx, res, carry, overflow);
    set_valor(vmx, tipo_a, dato_a, res);
    log_operacion_2("SUB", a, b, res, vmx->registros[CC]);
}

void op_mul(Vmx *vmx) {
    int tipo_a = get_tipo(vmx->registros[OP1]);
    int32_t dato_a = get_dato(vmx->registros[OP1]);
    int tipo_b = get_tipo(vmx->registros[OP2]);
    int32_t dato_b = get_dato(vmx->registros[OP2]);

    int32_t a = get_valor(vmx, tipo_a, dato_a);
    int32_t b = get_valor(vmx, tipo_b, dato_b);

    int64_t prod_con_signo = (int64_t)a * (int64_t)b;
    int overflow = (prod_con_signo < INT32_MIN || prod_con_signo > INT32_MAX);

    uint32_t op_a = a;
    uint32_t op_b = b;
    uint64_t prod_sin_signo = (uint64_t)op_a * op_b;
    int carry = (prod_sin_signo > UINT32_MAX);

    int32_t res = (int32_t)prod_con_signo;

    actualizar_cc(vmx, res, carry, overflow);
    set_valor(vmx, tipo_a, dato_a, res);
    log_operacion_2("MUL", a, b, res, vmx->registros[CC]);
}

void op_div(Vmx *vmx) {
    int tipo_a = get_tipo(vmx->registros[OP1]);
    int32_t dato_a = get_dato(vmx->registros[OP1]);
    int tipo_b = get_tipo(vmx->registros[OP2]);
    int32_t dato_b = get_dato(vmx->registros[OP2]);

    int32_t a = get_valor(vmx, tipo_a, dato_a);
    int32_t b = get_valor(vmx, tipo_b, dato_b);
    int32_t res;
    int32_t resto;
    int overflow = 0;

    if (b == 0) {
        logger("[ERROR][EXEC] Division por cero: dividendo=%d, divisor=0 en IP=0x%08X\n", a, vmx->registros[IP]);
        vmx->abortar("Error: Division por cero al ejecutar instruccion DIV.");
        return;
    }

    if (a == INT32_MIN && b == -1) {
        res = INT32_MIN;
        resto = 0;
        overflow = 1;
    } else {
        res = a / b;
        resto = a % b;
    }

    vmx->registros[AC] = resto;

    actualizar_cc(vmx, res, 0, overflow);
    set_valor(vmx, tipo_a, dato_a, res);

    logger("[EXEC] DIV: dividendo=%d, divisor=%d => cociente=%d (en destino), resto=%d (en AC) [N=%d Z=%d C=0 V=%d]\n",
           a, b, res, resto, FLAG_N(vmx->registros[CC]), FLAG_Z(vmx->registros[CC]), overflow);
}

void op_cmp(Vmx *vmx) {
    int tipo_a = get_tipo(vmx->registros[OP1]);
    int32_t dato_a = get_dato(vmx->registros[OP1]);
    int tipo_b = get_tipo(vmx->registros[OP2]);
    int32_t dato_b = get_dato(vmx->registros[OP2]);

    int32_t a = get_valor(vmx, tipo_a, dato_a);
    int32_t b = get_valor(vmx, tipo_b, dato_b);
    int32_t res = a - b;

    int carry = (a < b);

    int64_t resta_con_signo = (int64_t)a - (int64_t)b;
    int overflow = (resta_con_signo < INT32_MIN || resta_con_signo > INT32_MAX);

    actualizar_cc(vmx, res, carry, overflow);
    log_operacion_2("CMP", a, b, res, vmx->registros[CC]);
}

void op_and(Vmx *vmx) {
    int tipo_a = get_tipo(vmx->registros[OP1]);
    int32_t dato_a = get_dato(vmx->registros[OP1]);
    int tipo_b = get_tipo(vmx->registros[OP2]);
    int32_t dato_b = get_dato(vmx->registros[OP2]);

    int32_t a = get_valor(vmx, tipo_a, dato_a);
    int32_t b = get_valor(vmx, tipo_b, dato_b);
    int32_t res = a & b;

    actualizar_cc(vmx, res, 0, 0);
    set_valor(vmx, tipo_a, dato_a, res);
    log_operacion_2("AND", a, b, res, vmx->registros[CC]);
}

void op_or(Vmx *vmx) {
    int tipo_a = get_tipo(vmx->registros[OP1]);
    int32_t dato_a = get_dato(vmx->registros[OP1]);
    int tipo_b = get_tipo(vmx->registros[OP2]);
    int32_t dato_b = get_dato(vmx->registros[OP2]);

    int32_t a = get_valor(vmx, tipo_a, dato_a);
    int32_t b = get_valor(vmx, tipo_b, dato_b);
    int32_t res = a | b;

    actualizar_cc(vmx, res, 0, 0);
    set_valor(vmx, tipo_a, dato_a, res);
    log_operacion_2("OR", a, b, res, vmx->registros[CC]);
}

void op_xor(Vmx *vmx) {
    int tipo_a = get_tipo(vmx->registros[OP1]);
    int32_t dato_a = get_dato(vmx->registros[OP1]);
    int tipo_b = get_tipo(vmx->registros[OP2]);
    int32_t dato_b = get_dato(vmx->registros[OP2]);

    int32_t a = get_valor(vmx, tipo_a, dato_a);
    int32_t b = get_valor(vmx, tipo_b, dato_b);
    int32_t res = a ^ b;

    actualizar_cc(vmx, res, 0, 0);
    set_valor(vmx, tipo_a, dato_a, res);
    log_operacion_2("XOR", a, b, res, vmx->registros[CC]);
}

void op_swap(Vmx *vmx) {
    int tipo_a = get_tipo(vmx->registros[OP1]);
    int32_t dato_a = get_dato(vmx->registros[OP1]);
    int tipo_b = get_tipo(vmx->registros[OP2]);
    int32_t dato_b = get_dato(vmx->registros[OP2]);

    int32_t a = get_valor(vmx, tipo_a, dato_a);
    int32_t b = get_valor(vmx, tipo_b, dato_b);

    set_valor(vmx, tipo_a, dato_a, b);
    set_valor(vmx, tipo_b, dato_b, a);

    // SWAP afecta a CC de la misma manera que el ultimo XOR entre ellos
    actualizar_cc(vmx, a, 0, 0);
    logger("[EXEC] SWAP: intercambiados opA=%d y opB=%d [CC afectado como XOR: N=%d Z=%d C=0 V=0]\n",
           b, a, FLAG_N(vmx->registros[CC]), FLAG_Z(vmx->registros[CC]));
}

void op_shl(Vmx *vmx) {
    int tipo_a = get_tipo(vmx->registros[OP1]);
    int32_t dato_a = get_dato(vmx->registros[OP1]);
    int tipo_b = get_tipo(vmx->registros[OP2]);
    int32_t dato_b = get_dato(vmx->registros[OP2]);

    int32_t a = get_valor(vmx, tipo_a, dato_a);
    int32_t b = get_valor(vmx, tipo_b, dato_b);
    int32_t res = 0;
    int carry = 0;
    int overflow = 0;
    int i;
    uint32_t u_a = a;

    if (b > 0) {
        if (b >= 32) {
            carry = (a != 0);
            res = 0;
        } else {
            // Deteccion de acarreo bit a bit si se pierde algun bit en uno
            i = 0;
            while (i < b && carry == 0) {
                if ((u_a >> (31 - i)) & 1) {
                    carry = 1;
                }
                i++;
            }
            res = (int32_t)(u_a << b);
        }
        int64_t shift_64 = (int64_t)a << (b < 63 ? b : 63);
        overflow = (shift_64 < INT32_MIN || shift_64 > INT32_MAX);
    } else {
        res = a;
    }

    actualizar_cc(vmx, res, carry, overflow);
    set_valor(vmx, tipo_a, dato_a, res);
    log_operacion_2("SHL", a, b, res, vmx->registros[CC]);
}

void op_shr(Vmx *vmx) {
    int tipo_a = get_tipo(vmx->registros[OP1]);
    int32_t dato_a = get_dato(vmx->registros[OP1]);
    int tipo_b = get_tipo(vmx->registros[OP2]);
    int32_t dato_b = get_dato(vmx->registros[OP2]);

    int32_t a = get_valor(vmx, tipo_a, dato_a);
    int32_t b = get_valor(vmx, tipo_b, dato_b);
    int32_t res = 0;
    int carry = 0;
    int i;
    int n;
    int aux;
    uint32_t u_a = a;
    uint32_t temp;

    if (b > 0) {
        if (b >= 32) {
            carry = (a != 0);
            res = 0;
        } else {
            // Deteccion de acarreo bit a bit si se pierde algun bit en uno
            i = 0;
            while (i < b && carry == 0) {
                if ((u_a >> i) & 1) {
                    carry = 1;
                }
                i++;
            }
            // Reconstruccion del valor desplazado bit a bit
            n = 32 - b;
            temp = u_a >> b;
            for (i = 0; i < n; i++) {
                aux = temp & 1;
                aux = aux << i;
                res += aux;
                temp = temp >> 1;
            }
        }
    } else {
        res = a;
    }

    actualizar_cc(vmx, res, carry, 0);
    set_valor(vmx, tipo_a, dato_a, res);
    log_operacion_2("SHR", a, b, res, vmx->registros[CC]);
}

void op_sar(Vmx *vmx) {
    int tipo_a = get_tipo(vmx->registros[OP1]);
    int32_t dato_a = get_dato(vmx->registros[OP1]);
    int tipo_b = get_tipo(vmx->registros[OP2]);
    int32_t dato_b = get_dato(vmx->registros[OP2]);

    int32_t a = get_valor(vmx, tipo_a, dato_a);
    int32_t b = get_valor(vmx, tipo_b, dato_b);
    int32_t res = 0;
    int carry = 0;
    int i;
    uint32_t u_a = a;

    if (b > 0) {
        if (b >= 32) {
            carry = (a < 0) ? 1 : (a != 0);
            res = (a < 0) ? -1 : 0;
        } else {
            i = 0;
            while (i < b && carry == 0) {
                if ((u_a >> i) & 1) {
                    carry = 1;
                }
                i++;
            }
            res = a >> b;
        }
    } else {
        res = a;
    }

    actualizar_cc(vmx, res, carry, 0);
    set_valor(vmx, tipo_a, dato_a, res);
    log_operacion_2("SAR", a, b, res, vmx->registros[CC]);
}

void op_ldl(Vmx *vmx) {
    int tipo_a = get_tipo(vmx->registros[OP1]);
    int32_t dato_a = get_dato(vmx->registros[OP1]);
    int tipo_b = get_tipo(vmx->registros[OP2]);
    int32_t dato_b = get_dato(vmx->registros[OP2]);

    int32_t a = get_valor(vmx, tipo_a, dato_a);
    int32_t b = get_valor(vmx, tipo_b, dato_b);

    // LDL: carga los 16 bits bajos preservando los altos; no modifica CC
    combinar_mitad(vmx, tipo_a, dato_a, (uint16_t)(b & 0xFFFF), 0);
    logger("[EXEC] LDL: cargando 16 bits bajos (0x%04X) en destino (antiguo=0x%08X nuevo=0x%08X)\n",
           (uint16_t)(b & 0xFFFF), a, get_valor(vmx, tipo_a, dato_a));
}

void op_ldh(Vmx *vmx) {
    int tipo_a = get_tipo(vmx->registros[OP1]);
    int32_t dato_a = get_dato(vmx->registros[OP1]);
    int tipo_b = get_tipo(vmx->registros[OP2]);
    int32_t dato_b = get_dato(vmx->registros[OP2]);

    int32_t a = get_valor(vmx, tipo_a, dato_a);
    int32_t b = get_valor(vmx, tipo_b, dato_b);

    // LDH: carga los 16 bits altos preservando los bajos; no modifica CC
    combinar_mitad(vmx, tipo_a, dato_a, (uint16_t)(b & 0xFFFF), 1);
    logger("[EXEC] LDH: cargando 16 bits altos (0x%04X) en destino (antiguo=0x%08X nuevo=0x%08X)\n",
           (uint16_t)(b & 0xFFFF), a, get_valor(vmx, tipo_a, dato_a));
}

void op_rnd(Vmx *vmx) {
    int tipo_a = get_tipo(vmx->registros[OP1]);
    int32_t dato_a = get_dato(vmx->registros[OP1]);
    int tipo_b = get_tipo(vmx->registros[OP2]);
    int32_t dato_b = get_dato(vmx->registros[OP2]);

    int32_t b = get_valor(vmx, tipo_b, dato_b);
    int32_t res;

    // RND: numero aleatorio entre 0 y operando B; no modifica CC
    if (b <= 0) {
        res = 0;
    } else {
        res = rand() % (b + 1);
    }

    set_valor(vmx, tipo_a, dato_a, res);
    logger("[EXEC] RND: limite=%d => generado=%d (no afecta CC)\n", b, res);
}

// Registra todas las operaciones en la tabla de operaciones
void inicializar_operadores(void) {
    tabla_operaciones[OP_SYS]  = op_sys;
    tabla_operaciones[OP_JMP]  = op_jmp;
    tabla_operaciones[OP_JP]   = op_jp;
    tabla_operaciones[OP_JN]   = op_jn;
    tabla_operaciones[OP_JZ]   = op_jz;
    tabla_operaciones[OP_JC]   = op_jc;
    tabla_operaciones[OP_JV]   = op_jv;
    tabla_operaciones[OP_JNP]  = op_jnp;
    tabla_operaciones[OP_JNN]  = op_jnn;
    tabla_operaciones[OP_JNZ]  = op_jnz;
    tabla_operaciones[OP_NOT]  = op_not;
    tabla_operaciones[OP_STOP] = op_stop;

    tabla_operaciones[OP_MOV]  = op_mov;
    tabla_operaciones[OP_ADD]  = op_add;
    tabla_operaciones[OP_SUB]  = op_sub;
    tabla_operaciones[OP_MUL]  = op_mul;
    tabla_operaciones[OP_DIV]  = op_div;
    tabla_operaciones[OP_CMP]  = op_cmp;
    tabla_operaciones[OP_AND]  = op_and;
    tabla_operaciones[OP_OR]   = op_or;
    tabla_operaciones[OP_XOR]  = op_xor;
    tabla_operaciones[OP_SWAP] = op_swap;
    tabla_operaciones[OP_SHL]  = op_shl;
    tabla_operaciones[OP_SHR]  = op_shr;
    tabla_operaciones[OP_SAR]  = op_sar;
    tabla_operaciones[OP_LDL]  = op_ldl;
    tabla_operaciones[OP_LDH]  = op_ldh;
    tabla_operaciones[OP_RND]  = op_rnd;
}
