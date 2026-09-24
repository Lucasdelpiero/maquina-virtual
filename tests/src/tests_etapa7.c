#include "tests_helpers.h"
#include "../../src/disassembler.h"
#include "../../src/sys.h"
#include "../../src/operandos.h"
#include "../../src/errores.h"
#include <string.h>
#include <setjmp.h>

static jmp_buf s_jmp_abort_sys;
static int s_abort_sys_llamado = 0;

static void mock_abortar_sys(char mensaje[]) {
    (void)mensaje;
    s_abort_sys_llamado = 1;
    longjmp(s_jmp_abort_sys, 1);
}

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

    // 4. Desensamblado de instruccion completa en memoria usando desensamblar_en_direccion
    // Ejemplo de la especificacion: [0000] B1 00 0A 00 05 9B | ADD [DS+5], 10
    vmx.memoria.mem_principal[0] = 0xB1;
    vmx.memoria.mem_principal[1] = 0x00;
    vmx.memoria.mem_principal[2] = 0x0A;
    vmx.memoria.mem_principal[3] = 0x00;
    vmx.memoria.mem_principal[4] = 0x05;
    vmx.memoria.mem_principal[5] = 0x9B;

    int bytes_add = desensamblar_en_direccion(&vmx, 0, buffer, sizeof(buffer));
    ASSERT_EQUAL(bytes_add, 6, "desensamblar_en_direccion de ADD consume 6 bytes");
    ASSERT(strstr(buffer, "ADD [DS+5], 10") != NULL, "desensamblar_en_direccion genera 'ADD [DS+5], 10'");
    ASSERT(strcmp(buffer, "[0000] B1 00 0A 00 05 9B | ADD [DS+5], 10") == 0, "desensamblar_en_direccion coincide exactamente con la especificacion");

    // STOP en direccion 6
    vmx.memoria.mem_principal[6] = OP_STOP;
    int bytes_stop = desensamblar_en_direccion(&vmx, 6, buffer, sizeof(buffer));
    ASSERT_EQUAL(bytes_stop, 1, "desensamblar_en_direccion de STOP consume 1 byte");
    ASSERT(strstr(buffer, "STOP") != NULL, "desensamblar_en_direccion genera 'STOP'");
    ASSERT(strcmp(buffer, "[0006] 0F                | STOP") == 0, "desensamblar_en_direccion STOP formato exacto");

    // 5. Validaciones estrictas de llamadas al sistema (SYS)
    set_abortar_handler(mock_abortar_sys);

    // Configurar operando para SYS 2 (WRITE)
    vmx.registros[OP1] = (TIPO_INMEDIATO << 24) | 2;
    vmx.registros[EDX] = 0x00010000; // dentro de datos
    vmx.memoria.tabla_segmentos[1] = (100 << 16) | 1000; // base 100, limite 1000

    // SYS con mascara de modo 0 (invalido)
    vmx.registros[EAX] = 0x00;
    vmx.registros[ECX] = (4 << 16) | 1;
    s_abort_sys_llamado = 0;
    if (setjmp(s_jmp_abort_sys) == 0) {
        op_sys(&vmx);
    }
    ASSERT(s_abort_sys_llamado == 1, "SYS con modo en EAX = 0 aborta");

    // SYS con tamano = 0 en ECX (invalido)
    vmx.registros[EAX] = 0x01;
    vmx.registros[ECX] = (0 << 16) | 1;
    s_abort_sys_llamado = 0;
    if (setjmp(s_jmp_abort_sys) == 0) {
        op_sys(&vmx);
    }
    ASSERT(s_abort_sys_llamado == 1, "SYS con tamano <= 0 en ECX aborta");

    // SYS con tamano = 5 en ECX (invalido)
    vmx.registros[ECX] = (5 << 16) | 1;
    s_abort_sys_llamado = 0;
    if (setjmp(s_jmp_abort_sys) == 0) {
        op_sys(&vmx);
    }
    ASSERT(s_abort_sys_llamado == 1, "SYS con tamano > 4 en ECX aborta");

    // SYS con cantidad = 0 en ECX (invalido)
    vmx.registros[ECX] = (4 << 16) | 0;
    s_abort_sys_llamado = 0;
    if (setjmp(s_jmp_abort_sys) == 0) {
        op_sys(&vmx);
    }
    ASSERT(s_abort_sys_llamado == 1, "SYS con cantidad <= 0 en ECX aborta");

    set_abortar_handler(NULL);

    // 6. Test de actualizacion de registros de bus (LAR, MAR, MBR) en SYS 2 (WRITE)
    vmx.memoria.mem_principal[100] = 0x12;
    vmx.memoria.mem_principal[101] = 0x34;
    vmx.memoria.mem_principal[102] = 0x56;
    vmx.memoria.mem_principal[103] = 0x78;
    vmx.registros[OP1] = (TIPO_INMEDIATO << 24) | 2; // SYS 2 (WRITE)
    vmx.registros[EAX] = 0x01; // Decimal
    vmx.registros[EDX] = 0x00010000; // segmento 1, offset 0 -> dir física 100
    vmx.registros[ECX] = (4 << 16) | 1; // 1 valor de 4 bytes
    op_sys(&vmx);
    assert_registro(&vmx, LAR, 0x00010000, "SYS WRITE actualiza LAR con direccion logica");
    ASSERT_EQUAL((int)(vmx.registros[MAR] & 0xFFFF), 100, "SYS WRITE actualiza MAR con direccion fisica");
    ASSERT_EQUAL((int)((vmx.registros[MAR] >> 16) & 0xFFFF), 4, "SYS WRITE actualiza MAR con cant_bytes");
    assert_registro(&vmx, MBR, 0x12345678, "SYS WRITE actualiza MBR con el valor leido");
}
