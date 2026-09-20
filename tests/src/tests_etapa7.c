#include "tests_helpers.h"
#include "../../src/disassembler.h"
#include "../../src/sys.h"
#include "../../src/operandos.h"
#include <string.h>

void correr_tests_etapa7(void) {
    Vmx vmx;
    char buffer[128];

    printf("\n--- TESTS: DISASSEMBLER Y SYS ---\n");

    inicializar_vmx(&vmx, 0, 0);

    // 1. Mnemonicos
    ASSERT(strcmp(obtener_mnemonico(OP_SYS), "SYS") == 0, "Mnemonico OP_SYS es 'SYS'");
    ASSERT(strcmp(obtener_mnemonico(OP_MOV), "MOV") == 0, "Mnemonico OP_MOV es 'MOV'");
    ASSERT(strcmp(obtener_mnemonico(OP_ADD), "ADD") == 0, "Mnemonico OP_ADD es 'ADD'");
    ASSERT(strcmp(obtener_mnemonico(OP_STOP), "STOP") == 0, "Mnemonico OP_STOP es 'STOP'");
    ASSERT(strcmp(obtener_mnemonico(OP_LDH), "LDH") == 0, "Mnemonico OP_LDH es 'LDH'");
    ASSERT(strcmp(obtener_mnemonico(OP_LDL), "LDL") == 0, "Mnemonico OP_LDL es 'LDL'");

    // 2. Nombres de registros
    ASSERT(strcmp(obtener_nombre_registro(EAX), "EAX") == 0, "Registro EAX es 'EAX'");
    ASSERT(strcmp(obtener_nombre_registro(IP), "IP") == 0, "Registro IP es 'IP'");
    ASSERT(strcmp(obtener_nombre_registro(CS), "CS") == 0, "Registro CS es 'CS'");
    ASSERT(strcmp(obtener_nombre_registro(DS), "DS") == 0, "Registro DS es 'DS'");
    ASSERT(strcmp(obtener_nombre_registro(AC), "AC") == 0, "Registro AC es 'AC'");
    ASSERT(strcmp(obtener_nombre_registro(CC), "CC") == 0, "Registro CC es 'CC'");

    // 3. Formateo de operandos
    formatear_operando(buffer, TIPO_REGISTRO, EAX);
    ASSERT(strcmp(buffer, "EAX") == 0, "formatear_operando registro formatea 'EAX'");

    formatear_operando(buffer, TIPO_INMEDIATO, 15);
    ASSERT(strcmp(buffer, "15") == 0, "formatear_operando inmediato formatea '15'");

    // Memoria: registro DS con offset 10 -> (10 << 8) | DS
    formatear_operando(buffer, TIPO_MEMORIA, (10 << 8) | DS);
    ASSERT(strcmp(buffer, "[DS+10]") == 0, "formatear_operando memoria formatea '[DS+10]'");

    // 4. Desensamblado de instruccion completa en buffer
    vmx.registros[IP] = 0x00000000;
    vmx.registros[OPC] = OP_ADD;
    vmx.registros[OP1] = (TIPO_MEMORIA << 24) | ((5 << 8) | DS);
    vmx.registros[OP2] = (TIPO_INMEDIATO << 24) | 10;

    desensamblar_instruccion(&vmx, buffer, sizeof(buffer));
    ASSERT(strstr(buffer, "ADD [DS+5], 10") != NULL, "desensamblar_instruccion genera 'ADD [DS+5], 10'");

    // STOP
    vmx.registros[IP] = 0x00000010;
    vmx.registros[OPC] = OP_STOP;
    vmx.registros[OP1] = 0;
    vmx.registros[OP2] = 0;

    desensamblar_instruccion(&vmx, buffer, sizeof(buffer));
    ASSERT(strstr(buffer, "STOP") != NULL, "desensamblar_instruccion genera 'STOP'");
}
