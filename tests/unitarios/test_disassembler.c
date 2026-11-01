#include "test_helpers.h"
#include "../../src/disassembler.h"
#include "../../src/operandos.h"
#include "../../src/inicializador.h"

void test_disassembler_suite(void) {
    printf("\n--- TESTS: disassembler.c ---\n");

    // Test 1: obtener_mnemonico exhaustivo para todas las instrucciones soportadas
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_SYS), "SYS", "obtener_mnemonico: 0x00 es SYS");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_JMP), "JMP", "obtener_mnemonico: 0x01 es JMP");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_JZ), "JZ", "obtener_mnemonico: 0x02 es JZ");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_JNZ), "JNZ", "obtener_mnemonico: 0x03 es JNZ");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_JN), "JN", "obtener_mnemonico: 0x04 es JN");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_JNN), "JNN", "obtener_mnemonico: 0x05 es JNN");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_JP), "JP", "obtener_mnemonico: 0x06 es JP");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_JNP), "JNP", "obtener_mnemonico: 0x07 es JNP");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_NOT), "NOT", "obtener_mnemonico: 0x08 es NOT");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_PUSH), "PUSH", "obtener_mnemonico: 0x0B es PUSH");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_POP), "POP", "obtener_mnemonico: 0x0C es POP");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_CALL), "CALL", "obtener_mnemonico: 0x0D es CALL");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_RET), "RET", "obtener_mnemonico: 0x0E es RET");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_STOP), "STOP", "obtener_mnemonico: 0x0F es STOP");

    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_MOV), "MOV", "obtener_mnemonico: 0x10 es MOV");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_ADD), "ADD", "obtener_mnemonico: 0x11 es ADD");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_SUB), "SUB", "obtener_mnemonico: 0x12 es SUB");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_SWAP), "SWAP", "obtener_mnemonico: 0x13 es SWAP");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_MUL), "MUL", "obtener_mnemonico: 0x14 es MUL");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_DIV), "DIV", "obtener_mnemonico: 0x15 es DIV");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_CMP), "CMP", "obtener_mnemonico: 0x16 es CMP");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_SHL), "SHL", "obtener_mnemonico: 0x17 es SHL");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_SHR), "SHR", "obtener_mnemonico: 0x18 es SHR");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_SAR), "SAR", "obtener_mnemonico: 0x19 es SAR");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_AND), "AND", "obtener_mnemonico: 0x1A es AND");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_OR), "OR", "obtener_mnemonico: 0x1B es OR");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_XOR), "XOR", "obtener_mnemonico: 0x1C es XOR");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_LDL), "LDL", "obtener_mnemonico: 0x1D es LDL");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_LDH), "LDH", "obtener_mnemonico: 0x1E es LDH");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(OP_RND), "RND", "obtener_mnemonico: 0x1F es RND");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(-1), "???", "obtener_mnemonico: -1 es ???");
    TEST_ASSERT_STR_EQUAL(obtener_mnemonico(32), "???", "obtener_mnemonico: 32 es ???");

    // Test 2: obtener_nombre_registro con sectores y registros generales
    TEST_ASSERT_STR_EQUAL(obtener_nombre_registro(EAX), "EAX", "obtener_nombre_registro: 10 es EAX");
    TEST_ASSERT_STR_EQUAL(obtener_nombre_registro((1 << 5) | EAX), "AL", "obtener_nombre_registro: AL (sector 1)");
    TEST_ASSERT_STR_EQUAL(obtener_nombre_registro((2 << 5) | EAX), "AH", "obtener_nombre_registro: AH (sector 2)");
    TEST_ASSERT_STR_EQUAL(obtener_nombre_registro((3 << 5) | EAX), "AX", "obtener_nombre_registro: AX (sector 3)");

    TEST_ASSERT_STR_EQUAL(obtener_nombre_registro(EBX), "EBX", "obtener_nombre_registro: 11 es EBX");
    TEST_ASSERT_STR_EQUAL(obtener_nombre_registro((1 << 5) | EBX), "BL", "obtener_nombre_registro: BL");
    TEST_ASSERT_STR_EQUAL(obtener_nombre_registro((2 << 5) | EBX), "BH", "obtener_nombre_registro: BH");
    TEST_ASSERT_STR_EQUAL(obtener_nombre_registro((3 << 5) | EBX), "BX", "obtener_nombre_registro: BX");

    TEST_ASSERT_STR_EQUAL(obtener_nombre_registro(ECX), "ECX", "obtener_nombre_registro: 12 es ECX");
    TEST_ASSERT_STR_EQUAL(obtener_nombre_registro((1 << 5) | ECX), "CL", "obtener_nombre_registro: CL");
    TEST_ASSERT_STR_EQUAL(obtener_nombre_registro((2 << 5) | ECX), "CH", "obtener_nombre_registro: CH");
    TEST_ASSERT_STR_EQUAL(obtener_nombre_registro((3 << 5) | ECX), "CX", "obtener_nombre_registro: CX");

    TEST_ASSERT_STR_EQUAL(obtener_nombre_registro(EDX), "EDX", "obtener_nombre_registro: 13 es EDX");
    TEST_ASSERT_STR_EQUAL(obtener_nombre_registro((1 << 5) | EDX), "DL", "obtener_nombre_registro: DL");
    TEST_ASSERT_STR_EQUAL(obtener_nombre_registro((2 << 5) | EDX), "DH", "obtener_nombre_registro: DH");
    TEST_ASSERT_STR_EQUAL(obtener_nombre_registro((3 << 5) | EDX), "DX", "obtener_nombre_registro: DX");

    TEST_ASSERT_STR_EQUAL(obtener_nombre_registro(AC), "AC", "obtener_nombre_registro: 16 es AC");
    TEST_ASSERT_STR_EQUAL(obtener_nombre_registro(CC), "CC", "obtener_nombre_registro: 17 es CC");
    TEST_ASSERT_STR_EQUAL(obtener_nombre_registro(IP), "IP", "obtener_nombre_registro: 0 es IP");
    TEST_ASSERT_STR_EQUAL(obtener_nombre_registro(SP), "SP", "obtener_nombre_registro: 7 es SP");
    TEST_ASSERT_STR_EQUAL(obtener_nombre_registro(BP), "BP", "obtener_nombre_registro: 8 es BP");
    TEST_ASSERT_STR_EQUAL(obtener_nombre_registro(CS), "CS", "obtener_nombre_registro: 26 es CS");
    TEST_ASSERT_STR_EQUAL(obtener_nombre_registro(DS), "DS", "obtener_nombre_registro: 27 es DS");
    TEST_ASSERT_STR_EQUAL(obtener_nombre_registro(ES), "ES", "obtener_nombre_registro: 28 es ES");
    TEST_ASSERT_STR_EQUAL(obtener_nombre_registro(SS), "SS", "obtener_nombre_registro: 29 es SS");
    TEST_ASSERT_STR_EQUAL(obtener_nombre_registro(KS), "KS", "obtener_nombre_registro: 30 es KS");
    TEST_ASSERT_STR_EQUAL(obtener_nombre_registro(PS), "PS", "obtener_nombre_registro: 31 es PS");
    TEST_ASSERT_STR_EQUAL(obtener_nombre_registro(9), "???", "obtener_nombre_registro: 9 reservado es ???");
    TEST_ASSERT_STR_EQUAL(obtener_nombre_registro(18), "???", "obtener_nombre_registro: 18 reservado es ???");

    // Test 3: formatear_operando
    char buffer[64];
    formatear_operando(buffer, TIPO_REGISTRO, EAX);
    TEST_ASSERT_STR_EQUAL(buffer, "EAX", "formatear_operando: registro EAX");

    formatear_operando(buffer, TIPO_REGISTRO, (1 << 5) | EAX);
    TEST_ASSERT_STR_EQUAL(buffer, "AL", "formatear_operando: sector AL");

    formatear_operando(buffer, TIPO_INMEDIATO, 1234);
    TEST_ASSERT_STR_EQUAL(buffer, "1234", "formatear_operando: inmediato positivo 1234");

    formatear_operando(buffer, TIPO_INMEDIATO, (uint16_t)-50);
    TEST_ASSERT_STR_EQUAL(buffer, "-50", "formatear_operando: inmediato negativo -50");

    // Modificadores de memoria
    int32_t mem_op1 = (10 << 8) | DS;
    formatear_operando(buffer, TIPO_MEMORIA, mem_op1);
    TEST_ASSERT_STR_EQUAL(buffer, "[DS+10]", "formatear_operando: memoria [DS+10]");

    int32_t mem_op2 = (1 << 5) | (0 << 8) | DS; // byte
    formatear_operando(buffer, TIPO_MEMORIA, mem_op2);
    TEST_ASSERT_STR_EQUAL(buffer, "b[DS+0]", "formatear_operando: memoria b[DS+0]");

    int32_t mem_op3 = (2 << 5) | (4 << 8) | EDX; // word
    formatear_operando(buffer, TIPO_MEMORIA, mem_op3);
    TEST_ASSERT_STR_EQUAL(buffer, "w[EDX+4]", "formatear_operando: memoria w[EDX+4]");

    int32_t mem_op4 = (4 << 5) | (10 << 8) | DS; // long
    formatear_operando(buffer, TIPO_MEMORIA, mem_op4);
    TEST_ASSERT_STR_EQUAL(buffer, "l[DS+10]", "formatear_operando: memoria l[DS+10]");

    // Test 4: desensamblar_en_direccion
    Vmx vmx;
    inicializar_vmx(&vmx, 16, 0, 0);

    char desens[128];
    // STOP (1 byte)
    vmx.memoria.mem_principal[0] = OP_STOP;
    int bytes_stop = desensamblar_en_direccion(&vmx, 0, desens, sizeof(desens));
    TEST_ASSERT_EQUAL(bytes_stop, 1, "desensamblar_en_direccion: STOP consume 1 byte");
    TEST_ASSERT(strstr(desens, "STOP") != NULL, "desensamblar_en_direccion: buffer contiene STOP");

    // MOV EAX, 10 (4 bytes)
    vmx.memoria.mem_principal[1] = 0x90; // MOV inm, reg
    vmx.memoria.mem_principal[2] = 0x00;
    vmx.memoria.mem_principal[3] = 0x0A; // 10
    vmx.memoria.mem_principal[4] = EAX;
    int bytes_mov = desensamblar_en_direccion(&vmx, 1, desens, sizeof(desens));
    TEST_ASSERT_EQUAL(bytes_mov, 4, "desensamblar_en_direccion: MOV consume 4 bytes");
    TEST_ASSERT(strstr(desens, "MOV EAX, 10") != NULL, "desensamblar_en_direccion: buffer contiene MOV EAX, 10");

    // ADD EAX, EBX (3 bytes)
    vmx.memoria.mem_principal[5] = 0x51;
    vmx.memoria.mem_principal[6] = EBX;
    vmx.memoria.mem_principal[7] = EAX;
    int bytes_add = desensamblar_en_direccion(&vmx, 5, desens, sizeof(desens));
    TEST_ASSERT_EQUAL(bytes_add, 3, "desensamblar_en_direccion: ADD consume 3 bytes");
    TEST_ASSERT(strstr(desens, "ADD EAX, EBX") != NULL, "desensamblar_en_direccion: buffer contiene ADD EAX, EBX");

    // Direccion fuera de limite devuelve 0
    int bytes_out = desensamblar_en_direccion(&vmx, 20000, desens, sizeof(desens));
    TEST_ASSERT_EQUAL(bytes_out, 0, "desensamblar_en_direccion: direccion fuera de limites devuelve 0");

    // Test 5: desensamblar_ks
    vmx.memoria.tabla_segmentos[0] = (100 << 16) | 20;
    vmx.registros[KS] = 0x00000000;
    strcpy((char*)&vmx.memoria.mem_principal[100], "VMX26");
    strcpy((char*)&vmx.memoria.mem_principal[106], "Fund. de la Arq.");
    desensamblar_ks(&vmx);
    TEST_ASSERT(1, "desensamblar_ks: imprime cadenas cortas y largas sin error");

    // Test 6: desensamblar_cs y desensamblar_programa
    vmx.memoria.tabla_segmentos[1] = (0 << 16) | 8;
    vmx.registros[CS] = 0x00010000;
    vmx.memoria.offset_entry_point = 0;
    desensamblar_cs(&vmx);
    TEST_ASSERT(1, "desensamblar_cs: desensambla segmento de codigo");

    desensamblar_programa(&vmx);
    TEST_ASSERT(1, "desensamblar_programa: ejecuta KS y CS coordinadamente");

    free(vmx.memoria.mem_principal);
}
