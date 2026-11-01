#include "test_helpers.h"
#include "../../src/vmx.h"
#include "../../src/operadores.h"
#include "../../src/inicializador.h"

void test_vmx_suite(void) {
    printf("\n--- TESTS: vmx.c ---\n");

    Vmx vmx;
    inicializar_vmx(&vmx, 16, 0, 0);

    // Configuramos entrada 0: Base 0, Tamano 100 bytes (CS)
    // Configuramos entrada 1: Base 500, Tamano 32 bytes (SS)
    vmx.memoria.tabla_segmentos[0] = (0 << 16) | 100;
    vmx.memoria.tabla_segmentos[1] = (500 << 16) | 32;
    vmx.registros[CS] = 0x00000000;
    vmx.registros[SS] = 0x00010000;
    vmx.registros[SP] = 0x00010020; // offset 32 (tope)

    // Test 1: ip_valido
    vmx.registros[IP] = 0x00000000;
    TEST_ASSERT_EQUAL(ip_valido(&vmx), 1, "ip_valido: IP en 0 dentro de CS es valido");

    vmx.registros[IP] = 0x00000063; // offset 99 < 100
    TEST_ASSERT_EQUAL(ip_valido(&vmx), 1, "ip_valido: ultimo offset valido (99)");

    vmx.registros[IP] = 0x00000064; // offset 100 == tamano
    TEST_ASSERT_EQUAL(ip_valido(&vmx), 0, "ip_valido: offset 100 fuera de limite es invalido");

    vmx.registros[IP] = -1; // STOP
    TEST_ASSERT_EQUAL(ip_valido(&vmx), 0, "ip_valido: IP en -1 (STOP) detiene el ciclo");

    vmx.registros[IP] = 0x00010000; // segmento 1 != CS
    TEST_ASSERT_EQUAL(ip_valido(&vmx), 0, "ip_valido: segmento distinto a CS es invalido");

    // Test 2: ejecutar_instruccion con opcode invalido
    vmx.registros[OPC] = 31; // RND es valido
    FuncionOperacion op_backup = tabla_operaciones[OP_RND];
    tabla_operaciones[OP_RND] = NULL;
    TEST_ASSERT_ABORTA(ejecutar_instruccion(&vmx), "ejecutar_instruccion: funcion nula aborta");
    tabla_operaciones[OP_RND] = op_backup;

    vmx.registros[OPC] = -5;
    TEST_ASSERT_ABORTA(ejecutar_instruccion(&vmx), "ejecutar_instruccion: opcode negativo aborta");

    vmx.registros[OPC] = 33;
    TEST_ASSERT_ABORTA(ejecutar_instruccion(&vmx), "ejecutar_instruccion: opcode > 31 aborta");

    // Test 3: Programa aritmetico completo (MOV EAX, 15; ADD EAX, 10; STOP)
    // Instruccion 1: MOV EAX, 15 (4 bytes) en pos 0..3
    vmx.memoria.mem_principal[0] = 0x90; // MOV inm, reg
    vmx.memoria.mem_principal[1] = 0x00;
    vmx.memoria.mem_principal[2] = 0x0F; // 15
    vmx.memoria.mem_principal[3] = EAX;

    // Instruccion 2: ADD EAX, 10 (4 bytes) en pos 4..7
    vmx.memoria.mem_principal[4] = 0x91; // ADD inm, reg
    vmx.memoria.mem_principal[5] = 0x00;
    vmx.memoria.mem_principal[6] = 0x0A; // 10
    vmx.memoria.mem_principal[7] = EAX;

    // Instruccion 3: STOP (1 byte) en pos 8
    vmx.memoria.mem_principal[8] = OP_STOP;

    vmx.registros[IP] = 0x00000000;
    ejecutar_vmx(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], 25, "ejecutar_vmx aritmetica: 15 + 10 = 25");
    TEST_ASSERT_EQUAL(vmx.registros[IP], -1, "ejecutar_vmx: IP quedo en -1 tras STOP");

    // Test 4: Programa con subrutina (CALL y RET)
    // [0000] CALL 0x0004  (3 bytes: 0x8D, 0x00, 0x04) -> salta a subrutina en pos 4
    // [0003] STOP         (1 byte: 0x0F)
    // [0004] MOV EAX, 99  (4 bytes: 0x90, 0x00, 0x63, EAX)
    // [0008] RET          (1 byte: 0x0E)
    vmx.memoria.mem_principal[0] = 0x8D; // CALL inm (opcode 0x0D, tipo inm 10)
    vmx.memoria.mem_principal[1] = 0x00;
    vmx.memoria.mem_principal[2] = 0x04; // destino 4
    vmx.memoria.mem_principal[3] = OP_STOP;

    vmx.memoria.mem_principal[4] = 0x90; // MOV inm, reg
    vmx.memoria.mem_principal[5] = 0x00;
    vmx.memoria.mem_principal[6] = 0x63; // 99
    vmx.memoria.mem_principal[7] = EAX;
    vmx.memoria.mem_principal[8] = OP_RET;

    vmx.registros[IP] = 0x00000000;
    vmx.registros[SP] = 0x00010020; // restablecemos pila vacia
    vmx.registros[EAX] = 0;
    ejecutar_vmx(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], 99, "ejecutar_vmx CALL/RET: subrutina ejecuto y retorno EAX=99");
    TEST_ASSERT_EQUAL(vmx.registros[SP], 0x00010020, "ejecutar_vmx CALL/RET: SP restablecido al valor inicial");
    TEST_ASSERT_EQUAL(vmx.registros[IP], -1, "ejecutar_vmx CALL/RET: finalizo en STOP");

    // Test 5: Programa con bifurcacion condicional JZ
    // [0000] MOV EAX, 5  (0x90, 0x00, 0x05, EAX)
    // [0004] CMP EAX, 5  (0x96, 0x00, 0x05, EAX) -> activa Z=1
    // [0008] JZ 0x0010   (0x82, 0x00, 0x10)       -> salta a 16
    // [000B] MOV EBX, 1  (0x90, 0x00, 0x01, EBX)  -> camino falso (no debe ejecutarse)
    // [000F] STOP
    // [0010] MOV EBX, 2  (0x90, 0x00, 0x02, EBX)  -> camino verdadero
    // [0014] STOP
    vmx.memoria.mem_principal[0] = 0x90;
    vmx.memoria.mem_principal[1] = 0x00;
    vmx.memoria.mem_principal[2] = 0x05;
    vmx.memoria.mem_principal[3] = EAX;

    vmx.memoria.mem_principal[4] = 0x96; // CMP inm, reg (0x16)
    vmx.memoria.mem_principal[5] = 0x00;
    vmx.memoria.mem_principal[6] = 0x05;
    vmx.memoria.mem_principal[7] = EAX;

    vmx.memoria.mem_principal[8] = 0x82; // JZ inm (0x02)
    vmx.memoria.mem_principal[9] = 0x00;
    vmx.memoria.mem_principal[10] = 0x10; // offset 16

    vmx.memoria.mem_principal[11] = 0x90;
    vmx.memoria.mem_principal[12] = 0x00;
    vmx.memoria.mem_principal[13] = 0x01;
    vmx.memoria.mem_principal[14] = EBX;
    vmx.memoria.mem_principal[15] = OP_STOP;

    vmx.memoria.mem_principal[16] = 0x90;
    vmx.memoria.mem_principal[17] = 0x00;
    vmx.memoria.mem_principal[18] = 0x02;
    vmx.memoria.mem_principal[19] = EBX;
    vmx.memoria.mem_principal[20] = OP_STOP;

    vmx.registros[IP] = 0x00000000;
    vmx.registros[EBX] = 0;
    ejecutar_vmx(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EBX], 2, "ejecutar_vmx condicional: JZ tomo camino correcto asignando EBX=2");

    free(vmx.memoria.mem_principal);
}
