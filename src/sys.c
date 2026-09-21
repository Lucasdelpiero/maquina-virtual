#include "sys.h"
#include "operandos.h"
#include "memoria.h"
#include "logger.h"
#include "errores.h"
#include <stdio.h>
#include <stdint.h>

// Formatea y muestra un valor por consola segun la mascara de formatos de EAX
static void mostrar_valor_sys(int32_t valor, int tamanio, int formato) {
    int i;
    uint32_t numero;
    int mostrado;

    numero = (uint32_t)valor;
    mostrado = 0;

    // Bit 4 (0x10): Binario con prefijo 0b
    if (formato & 0x10) {
        if (mostrado) {
            printf(" ");
        }
        printf("0b");
        if (numero == 0) {
            printf("0");
        } else {
            int bit_alto = (tamanio * 8) - 1;
            while (bit_alto > 0 && (((numero >> bit_alto) & 1) == 0)) {
                bit_alto--;
            }
            for (i = bit_alto; i >= 0; i--) {
                printf("%d", (int)((numero >> i) & 1));
            }
        }
        mostrado = 1;
    }

    // Bit 3 (0x08): Hexadecimal con prefijo 0x
    if (formato & 0x08) {
        if (mostrado) {
            printf(" ");
        }
        printf("0x%X", numero);
        mostrado = 1;
    }

    // Bit 2 (0x04): Octal con prefijo 0o
    if (formato & 0x04) {
        if (mostrado) {
            printf(" ");
        }
        printf("0o%o", numero);
        mostrado = 1;
    }

    // Bit 1 (0x02): Caracteres (orden big-endian, no imprimibles como '.')
    if (formato & 0x02) {
        if (mostrado) {
            printf(" ");
        }
        for (i = tamanio - 1; i >= 0; i--) {
            char c = (char)((numero >> (8 * i)) & 0xFF);
            if (c < 32 || c > 126) {
                c = '.';
            }
            printf("%c", c);
        }
        mostrado = 1;
    }

    // Bit 0 (0x01): Decimal
    if (formato & 0x01) {
        if (mostrado) {
            printf(" ");
        }
        printf("%d", valor);
        mostrado = 1;
    }

    printf("\n");
}

// Lee un valor desde teclado segun la mascara de formatos de EAX
static int32_t leer_numero_sys(int formato) {
    uint32_t unum = 0;
    int num = 0;
    char c;
    char binario[36];
    int i;

    if (formato & 0x01) {
        // Decimal con signo
        scanf("%d", &num);
        return num;
    } else if (formato & 0x08) {
        // Hexadecimal
        scanf("%x", &unum);
        return (int32_t)unum;
    } else if (formato & 0x04) {
        // Octal
        scanf("%o", &unum);
        return (int32_t)unum;
    } else if (formato & 0x02) {
        // Caracter
        scanf(" %c", &c);
        return (uint8_t)c;
    } else if (formato & 0x10) {
        // Binario
        scanf("%35s", binario);
        unum = 0;
        i = 0;
        while (binario[i] != '\0') {
            unum = (unum << 1);
            if (binario[i] == '1') {
                unum |= 1;
            }
            i++;
        }
        return (int32_t)unum;
    }

    logger("[ERROR][SYS] Formato de lectura en EAX=0x%X no pertenece a ningun tipo de dato valido\n", formato);
    abortar("Error: Formato de lectura SYS invalido.");
    return 0;
}

void op_sys(Vmx *vmx) {
    int tipo_a;
    int32_t dato_a;
    int32_t num_sys;
    int modo;
    int32_t dir_base;
    int cantidad;
    int tamanio;
    int i;
    int32_t dir_logica;
    uint16_t dir_fisica;
    int32_t valor;

    tipo_a = get_tipo(vmx->registros[OP1]);
    dato_a = get_dato(vmx->registros[OP1]);
    num_sys = get_valor(vmx, tipo_a, dato_a);

    modo = vmx->registros[EAX];
    dir_base = vmx->registros[EDX];
    cantidad = vmx->registros[ECX] & 0xFFFF;
    tamanio = (vmx->registros[ECX] >> 16) & 0xFFFF;

    if (tamanio <= 0 || tamanio > 4) {
        logger("[ERROR][SYS] Tamano invalido en ECX: %d (debe ser entre 1 y 4 bytes)\n", tamanio);
        vmx->abortar("Error: Tamano invalido en SYS.");
        return;
    }
    if (cantidad <= 0) {
        logger("[ERROR][SYS] Cantidad invalida en ECX: %d (debe ser mayor a cero)\n", cantidad);
        vmx->abortar("Error: Cantidad invalida en SYS.");
        return;
    }

    if ((modo & 0x1F) == 0) {
        logger("[ERROR][SYS] Mascara de formato en EAX=0x%X invalida (ningun formato valido en bits 0..4)\n", modo);
        vmx->abortar("Error: Formato de SYS invalido.");
        return;
    }

    logger("[SYS] Invocando syscall %d: Modo=0x%X, DirBase=0x%08X, Cantidad=%d, Tamano=%d\n",
           num_sys, modo, dir_base, cantidad, tamanio);

    if (num_sys == 1) {
        // SYS 1: READ
        for (i = 0; i < cantidad; i++) {
            dir_logica = dir_base + (i * tamanio);
            dir_fisica = traducir_direccion(&vmx->memoria, dir_logica, tamanio);

            printf("[%04X]: ", dir_fisica);
            valor = leer_numero_sys(modo);

            escribir_memoria(&vmx->memoria, dir_fisica, tamanio, valor);
            set_registros_memoria(&vmx->memoria, dir_logica, tamanio, dir_fisica, valor);
            logger("[SYS] READ celda %d: DirLogica=0x%08X DirFisica=0x%04X Valor=%d (0x%X)\n",
                   i, dir_logica, dir_fisica, valor, valor);
        }
    } else if (num_sys == 2) {
        // SYS 2: WRITE
        for (i = 0; i < cantidad; i++) {
            dir_logica = dir_base + (i * tamanio);
            dir_fisica = traducir_direccion(&vmx->memoria, dir_logica, tamanio);

            valor = leer_memoria(&vmx->memoria, dir_fisica, tamanio);
            set_registros_memoria(&vmx->memoria, dir_logica, tamanio, dir_fisica, valor);

            printf("[%04X]: ", dir_fisica);
            mostrar_valor_sys(valor, tamanio, modo);

            logger("[SYS] WRITE celda %d: DirLogica=0x%08X DirFisica=0x%04X Valor=%d\n",
                   i, dir_logica, dir_fisica, valor);
        }
    } else {
        logger("[ERROR][SYS] Llamada al sistema invalida o no implementada: num_sys=%d (solo se soportan 1=READ y 2=WRITE)\n", num_sys);
        vmx->abortar("Error: Llamada al sistema (SYS) invalida o no implementada.");
    }
}
