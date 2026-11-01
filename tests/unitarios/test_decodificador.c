#include "test_helpers.h"
#include "../../src/decodificador.h"
#include "../../src/inicializador.h"

void test_decodificador_suite(void) {
    printf("\n--- TESTS: decodificador.c ---\n");

    Vmx vmx;
    inicializar_vmx(&vmx, 16, 0, 0);

    // Configuramos Code Segment en entrada 0: Base 0, Tamano 200 bytes
    vmx.memoria.tabla_segmentos[0] = (0 << 16) | 200;
    vmx.registros[CS] = 0x00000000;

    // Test 1: Instruccion sin operandos: STOP (0x0F)
    vmx.registros[IP] = 0x00000000;
    vmx.memoria.mem_principal[0] = OP_STOP; // 0x0F
    decodificar_instruccion(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[OPC], OP_STOP, "decodificar_instruccion: STOP detectado");
    TEST_ASSERT_EQUAL(vmx.registros[OP1], 0, "decodificar_instruccion: STOP OP1 es 0");
    TEST_ASSERT_EQUAL(vmx.registros[OP2], 0, "decodificar_instruccion: STOP OP2 es 0");
    TEST_ASSERT_EQUAL(vmx.registros[IP], 0x00000001, "decodificar_instruccion: STOP avanzo IP 1 byte");

    // Test 2: Instruccion sin operandos: RET (0x0E)
    vmx.registros[IP] = 0x00000001;
    vmx.memoria.mem_principal[1] = OP_RET; // 0x0E
    decodificar_instruccion(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[OPC], OP_RET, "decodificar_instruccion: RET detectado");
    TEST_ASSERT_EQUAL(vmx.registros[IP], 0x00000002, "decodificar_instruccion: RET avanzo IP 1 byte");

    // Test 3: Instruccion con 1 operando registro: NOT EAX (0x0A)
    // Primer byte: [tipo A = 01][00][opcode = 0x0A] -> 0b01001010 = 0x4A
    // Byte operando: EAX (10 = 0x0A)
    vmx.registros[IP] = 0x00000002;
    vmx.memoria.mem_principal[2] = 0x4A;
    vmx.memoria.mem_principal[3] = EAX;
    decodificar_instruccion(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[OPC], OP_NOT, "decodificar_instruccion: NOT opcode 0x0A");
    TEST_ASSERT_EQUAL(vmx.registros[OP1], (1 << 24) | EAX, "decodificar_instruccion: NOT OP1 contiene registro EAX");
    TEST_ASSERT_EQUAL(vmx.registros[OP2], 0, "decodificar_instruccion: NOT OP2 es 0");
    TEST_ASSERT_EQUAL(vmx.registros[IP], 0x00000004, "decodificar_instruccion: NOT avanzo IP 2 bytes");

    // Test 4: Instruccion con 1 operando sector de registro: PUSH AX (sector 3 = 11)
    // Primer byte: [tipo A = 01][00][opcode = 0x0B] -> 0b01001011 = 0x4B
    // Byte operando: (3 << 5) | EAX
    vmx.registros[IP] = 0x00000004;
    vmx.memoria.mem_principal[4] = 0x4B;
    vmx.memoria.mem_principal[5] = (3 << 5) | EAX;
    decodificar_instruccion(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[OPC], OP_PUSH, "decodificar_instruccion: PUSH opcode 0x0B");
    TEST_ASSERT_EQUAL(vmx.registros[OP1], (1 << 24) | (3 << 5) | EAX, "decodificar_instruccion: PUSH OP1 contiene AX");
    TEST_ASSERT_EQUAL(vmx.registros[IP], 0x00000006, "decodificar_instruccion: PUSH AX avanzo IP 2 bytes");

    // Test 5: Instruccion con 1 operando inmediato: JMP 0x0120 (0x01)
    // Primer byte: [tipo A = 10][00][opcode = 0x01] -> 0b10000001 = 0x81
    // Bytes operando: 0x01, 0x20
    vmx.registros[IP] = 0x00000006;
    vmx.memoria.mem_principal[6] = 0x81;
    vmx.memoria.mem_principal[7] = 0x01;
    vmx.memoria.mem_principal[8] = 0x20;
    decodificar_instruccion(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[OPC], OP_JMP, "decodificar_instruccion: JMP opcode 0x01");
    TEST_ASSERT_EQUAL(vmx.registros[OP1], (2 << 24) | 0x0120, "decodificar_instruccion: JMP OP1 contiene inmediato 0x0120");
    TEST_ASSERT_EQUAL(vmx.registros[IP], 0x00000009, "decodificar_instruccion: JMP avanzo IP 3 bytes");

    // Test 6: Instruccion con 1 operando memoria: PUSH [DS+4] (0x0B)
    // Primer byte: [tipo A = 11][00][opcode = 0x0B] -> 0b11001011 = 0xCB
    // Bytes operando: offset 0x00, 0x04, reg DS
    vmx.registros[IP] = 0x00000009;
    vmx.memoria.mem_principal[9] = 0xCB;
    vmx.memoria.mem_principal[10] = 0x00;
    vmx.memoria.mem_principal[11] = 0x04;
    vmx.memoria.mem_principal[12] = DS;
    decodificar_instruccion(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[OPC], OP_PUSH, "decodificar_instruccion: PUSH opcode 0x0B");
    TEST_ASSERT_EQUAL(vmx.registros[OP1], (3 << 24) | (4 << 8) | DS, "decodificar_instruccion: PUSH OP1 contiene memoria [DS+4]");
    TEST_ASSERT_EQUAL(vmx.registros[IP], 0x0000000D, "decodificar_instruccion: PUSH avanzo IP 4 bytes");

    // Test 7: Instruccion 2 operandos: MOV EAX, 10 (reg, inm -> 4 bytes)
    // Primer byte: [tipo B = 10][tipo A = 01][1][opcode = 0] -> 0b10010000 = 0x90
    // Operando B (inm 10): 0x00, 0x0A
    // Operando A (reg EAX): 0x0A
    vmx.registros[IP] = 0x0000000D;
    vmx.memoria.mem_principal[13] = 0x90;
    vmx.memoria.mem_principal[14] = 0x00;
    vmx.memoria.mem_principal[15] = 0x0A;
    vmx.memoria.mem_principal[16] = EAX;
    decodificar_instruccion(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[OPC], OP_MOV, "decodificar_instruccion: MOV opcode 0x10");
    TEST_ASSERT_EQUAL(vmx.registros[OP1], (1 << 24) | EAX, "decodificar_instruccion: MOV OP1 es EAX");
    TEST_ASSERT_EQUAL(vmx.registros[OP2], (2 << 24) | 10, "decodificar_instruccion: MOV OP2 es inmediato 10");
    TEST_ASSERT_EQUAL(vmx.registros[IP], 0x00000011, "decodificar_instruccion: MOV reg, inm avanzo IP 4 bytes");

    // Test 8: Instruccion 2 operandos: ADD EAX, EBX (reg, reg -> 3 bytes)
    // Primer byte: [tipo B = 01][tipo A = 01][1][opcode = 1] -> 0b01010001 = 0x51
    // Operando B: EBX (11)
    // Operando A: EAX (10)
    vmx.registros[IP] = 0x00000011;
    vmx.memoria.mem_principal[17] = 0x51;
    vmx.memoria.mem_principal[18] = EBX;
    vmx.memoria.mem_principal[19] = EAX;
    decodificar_instruccion(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[OPC], OP_ADD, "decodificar_instruccion: ADD opcode 0x11");
    TEST_ASSERT_EQUAL(vmx.registros[OP1], (1 << 24) | EAX, "decodificar_instruccion: ADD OP1 es EAX");
    TEST_ASSERT_EQUAL(vmx.registros[OP2], (1 << 24) | EBX, "decodificar_instruccion: ADD OP2 es EBX");
    TEST_ASSERT_EQUAL(vmx.registros[IP], 0x00000014, "decodificar_instruccion: ADD reg, reg avanzo IP 3 bytes");

    // Test 9: Instruccion 2 operandos memoria-memoria: MOV w[DS+2], l[DS+10] (7 bytes)
    // Primer byte: [tipo B = 11][tipo A = 11][1][opcode = 0] -> 0b11110000 = 0xF0
    // Operando B: l[DS+10] -> offset 0x000A, reg = (4 << 5) | DS
    // Operando A: w[DS+2]  -> offset 0x0002, reg = (2 << 5) | DS
    vmx.registros[IP] = 0x00000014;
    vmx.memoria.mem_principal[20] = 0xF0;
    // Operando B (pos 21..23)
    vmx.memoria.mem_principal[21] = 0x00;
    vmx.memoria.mem_principal[22] = 0x0A;
    vmx.memoria.mem_principal[23] = (4 << 5) | DS;
    // Operando A (pos 24..26)
    vmx.memoria.mem_principal[24] = 0x00;
    vmx.memoria.mem_principal[25] = 0x02;
    vmx.memoria.mem_principal[26] = (2 << 5) | DS;
    decodificar_instruccion(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[OPC], OP_MOV, "decodificar_instruccion: MOV mem, mem opcode 0x10");
    TEST_ASSERT_EQUAL(vmx.registros[OP1], (3 << 24) | (2 << 8) | ((2 << 5) | DS), "decodificar_instruccion: OP1 contiene w[DS+2]");
    TEST_ASSERT_EQUAL(vmx.registros[OP2], (3 << 24) | (10 << 8) | ((4 << 5) | DS), "decodificar_instruccion: OP2 contiene l[DS+10]");
    TEST_ASSERT_EQUAL(vmx.registros[IP], 0x0000001B, "decodificar_instruccion: MOV mem, mem avanzo IP 7 bytes (20 a 27)");

    // Test 10: Instruccion 2 operandos memoria-registro: MOV b[DS+0], AH (5 bytes)
    // Primer byte: [tipo B = 01 (AH)][tipo A = 11 (b[DS+0])][1][opcode = 0] -> 0b01110000 = 0x70
    // Operando B (AH): (2 << 5) | EAX
    // Operando A (b[DS+0]): 0x0000, (1 << 5) | DS
    vmx.registros[IP] = 0x0000001B;
    vmx.memoria.mem_principal[27] = 0x70;
    vmx.memoria.mem_principal[28] = (2 << 5) | EAX;
    vmx.memoria.mem_principal[29] = 0x00;
    vmx.memoria.mem_principal[30] = 0x00;
    vmx.memoria.mem_principal[31] = (1 << 5) | DS;
    decodificar_instruccion(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[OPC], OP_MOV, "decodificar_instruccion: MOV mem, reg opcode 0x10");
    TEST_ASSERT_EQUAL(vmx.registros[OP1], (3 << 24) | (0 << 8) | ((1 << 5) | DS), "decodificar_instruccion: OP1 es b[DS+0]");
    TEST_ASSERT_EQUAL(vmx.registros[OP2], (1 << 24) | ((2 << 5) | EAX), "decodificar_instruccion: OP2 es AH");
    TEST_ASSERT_EQUAL(vmx.registros[IP], 0x00000020, "decodificar_instruccion: MOV mem, reg avanzo IP 5 bytes (27 a 32)");

    // Test 11: Error al decodificar fuera de segmento (offset == tam_codigo)
    vmx.registros[IP] = 0x000000C8; // offset 200 == tam_codigo (fuera de rango 0..199)
    TEST_ASSERT_ABORTA(decodificar_instruccion(&vmx), "decodificar_instruccion: IP fuera de segmento aborta");

    free(vmx.memoria.mem_principal);
}
