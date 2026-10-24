#ifndef VMX_H
#define VMX_H

#include <stdint.h>
#include "memoria.h"

typedef struct Vmx Vmx;

// Codigos numericos de los 32 registros de la maquina virtual
typedef enum {
    IP = 0, OPC = 1, OP1 = 2, OP2 = 3, LAR = 4, MAR = 5, MBR = 6,
    SP = 7, BP = 8,
    // 9: reservado
    EAX = 10, EBX = 11, ECX = 12, EDX = 13, EEX = 14, EFX = 15,
    AC = 16, CC = 17,
    // 18-25: reservados
    CS = 26, DS = 27, ES = 28, SS = 29, KS = 30, PS = 31
} CodigoRegistro;

// Codigos de operacion de las instrucciones (26 operaciones de la especificacion)
typedef enum {
    OP_SYS = 0x00, OP_JMP = 0x01, OP_JP = 0x02, OP_JN = 0x03, OP_JZ = 0x04,
    OP_JC = 0x05, OP_JV = 0x06, OP_JNP = 0x07, OP_JNN = 0x08, OP_JNZ = 0x09,
    OP_NOT = 0x0A, OP_PUSH = 0X0B , OP_POP = 0X0C , OP_CALL = 0X0D ,
    OP_RET = 0X0E , OP_STOP = 0x0F,
    OP_MOV = 0x10, OP_ADD = 0x11, OP_SUB = 0x12, OP_MUL = 0x13, OP_DIV = 0x14,
    OP_CMP = 0x15, OP_AND = 0x16, OP_OR = 0x17, OP_XOR = 0x18, OP_SWAP = 0x19,
    OP_SHL = 0x1A, OP_SHR = 0x1B, OP_SAR = 0x1C, OP_LDL = 0x1D, OP_LDH = 0x1E,
    OP_RND = 0x1F
} CodigoOperacion;

// Firma uniforme para todas las operaciones de la maquina virtual
typedef void (*FuncionOperacion)(Vmx *vmx);

// Tabla de operaciones/funciones indexada por codigo de operacion (0..31)
extern FuncionOperacion tabla_operaciones[32];

struct Vmx {
    int32_t registros[32];
    Memoria memoria;
    int modo_debug;
    int modo_disassembler;
    void (*abortar)(char mensaje[]);
};

void inicializar_vmx(Vmx *vmx, int tam_mem_kib, int modo_debug, int modo_disassembler);
//void cargar_programa(Vmx *vmx, char ruta_archivo[], int tam_parametros);
int ip_valido(Vmx *vmx);
void ejecutar_instruccion(Vmx *vmx);
void ejecutar_vmx(Vmx *vmx);

#endif // VMX_H
