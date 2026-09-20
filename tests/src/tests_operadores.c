#include "tests_helpers.h"
#include "../../src/operadores.h"
#include "../../src/operandos.h"

// Helper para configurar OP1 u OP2 en registros
static void set_operando_reg(Vmx *vmx, int reg_op, int tipo, int32_t dato) {
    vmx->registros[reg_op] = (tipo << 24) | (dato & 0x00FFFFFF);
}

void correr_tests_operadores(void) {
    Vmx vmx;
    printf("\n--- TESTS DE OPERADORES Y CODIGO DE CONDICION ---\n");

    inicializar_vmx(&vmx, 0, 0);

    // 1. Caso de prueba especificacion: MOV EAX, -1 -> N=1, Z=0, C=0, V=0
    set_operando_reg(&vmx, OP1, TIPO_REGISTRO, EAX);
    set_operando_reg(&vmx, OP2, TIPO_INMEDIATO, 0xFFFF); // -1 extendido
    op_mov(&vmx);
    assert_registro(&vmx, EAX, -1, "MOV EAX, -1 asigna -1 correctamente");
    assert_cc(&vmx, 1, 0, 0, 0, "MOV EAX, -1 fija flags N=1 Z=0 C=0 V=0");

    // 2. Caso de prueba especificacion: ADD 2147483647, 2147483647 -> res=-2, N=1, Z=0, C=0, V=1
    vmx.registros[EAX] = 2147483647;
    vmx.registros[EBX] = 2147483647;
    set_operando_reg(&vmx, OP1, TIPO_REGISTRO, EAX);
    set_operando_reg(&vmx, OP2, TIPO_REGISTRO, EBX);
    op_add(&vmx);
    assert_registro(&vmx, EAX, -2, "ADD 2147483647, 2147483647 da -2");
    assert_cc(&vmx, 1, 0, 0, 1, "ADD produce overflow con signo V=1, C=0");

    // 3. Caso de prueba especificacion: SUB -3, 5 -> res=-8, N=1, Z=0, C=1, V=0
    vmx.registros[EAX] = -3;
    vmx.registros[EBX] = 5;
    set_operando_reg(&vmx, OP1, TIPO_REGISTRO, EAX);
    set_operando_reg(&vmx, OP2, TIPO_REGISTRO, EBX);
    op_sub(&vmx);
    assert_registro(&vmx, EAX, -8, "SUB -3, 5 da -8");
    assert_cc(&vmx, 1, 0, 1, 0, "SUB -3, 5 fija N=1, Z=0, C=1 (borrow), V=0");

    // 4. Caso de prueba especificacion: MUL 1073741827, 4 -> res=12, N=0, Z=0, C=1, V=1
    vmx.registros[EAX] = 1073741827;
    vmx.registros[EBX] = 4;
    set_operando_reg(&vmx, OP1, TIPO_REGISTRO, EAX);
    set_operando_reg(&vmx, OP2, TIPO_REGISTRO, EBX);
    op_mul(&vmx);
    assert_registro(&vmx, EAX, 12, "MUL 1073741827, 4 da 12");
    assert_cc(&vmx, 0, 0, 1, 1, "MUL 1073741827, 4 fija N=0, Z=0, C=1, V=1");

    // 5. Caso de prueba especificacion: DIV 6, 7 -> res=0, resto en AC=6, N=0, Z=1, C=0, V=0
    vmx.registros[EAX] = 6;
    vmx.registros[EBX] = 7;
    set_operando_reg(&vmx, OP1, TIPO_REGISTRO, EAX);
    set_operando_reg(&vmx, OP2, TIPO_REGISTRO, EBX);
    op_div(&vmx);
    assert_registro(&vmx, EAX, 0, "DIV 6, 7 da cociente 0");
    assert_registro(&vmx, AC, 6, "DIV 6, 7 guarda resto 6 en AC");
    assert_cc(&vmx, 0, 1, 0, 0, "DIV 6, 7 fija Z=1, N=0, C=0, V=0");

    // 6. Caso de prueba especificacion: SHL 1073741824, 2 -> res=0, N=0, Z=1, C=1, V=1
    vmx.registros[EAX] = 1073741824;
    vmx.registros[EBX] = 2;
    set_operando_reg(&vmx, OP1, TIPO_REGISTRO, EAX);
    set_operando_reg(&vmx, OP2, TIPO_REGISTRO, EBX);
    op_shl(&vmx);
    assert_registro(&vmx, EAX, 0, "SHL 1073741824, 2 da 0");
    assert_cc(&vmx, 0, 1, 1, 1, "SHL 1073741824, 2 fija N=0, Z=1, C=1, V=1");

    // 7. Test CMP: no modifica operando A
    vmx.registros[EAX] = 10;
    vmx.registros[EBX] = 10;
    set_operando_reg(&vmx, OP1, TIPO_REGISTRO, EAX);
    set_operando_reg(&vmx, OP2, TIPO_REGISTRO, EBX);
    op_cmp(&vmx);
    assert_registro(&vmx, EAX, 10, "CMP preserva el valor de operando A");
    assert_cc(&vmx, 0, 1, 0, 0, "CMP 10, 10 fija Z=1 (iguales)");

    // 8. Test AND, OR, XOR
    vmx.registros[EAX] = 0x0F0F;
    vmx.registros[EBX] = 0x00FF;
    set_operando_reg(&vmx, OP1, TIPO_REGISTRO, EAX);
    set_operando_reg(&vmx, OP2, TIPO_REGISTRO, EBX);
    op_and(&vmx);
    assert_registro(&vmx, EAX, 0x000F, "AND 0x0F0F, 0x00FF da 0x000F");

    op_or(&vmx);
    assert_registro(&vmx, EAX, 0x00FF, "OR 0x000F, 0x00FF da 0x00FF");

    op_xor(&vmx);
    assert_registro(&vmx, EAX, 0x0000, "XOR 0x00FF, 0x00FF da 0x0000");
    assert_cc(&vmx, 0, 1, 0, 0, "XOR con resultado cero fija Z=1");

    // 9. Test NOT
    vmx.registros[EAX] = 0;
    set_operando_reg(&vmx, OP1, TIPO_REGISTRO, EAX);
    op_not(&vmx);
    assert_registro(&vmx, EAX, -1, "NOT 0 da -1 (0xFFFFFFFF)");
    assert_cc(&vmx, 1, 0, 0, 0, "NOT 0 fija N=1");

    // 10. Test SWAP
    vmx.registros[EAX] = 100;
    vmx.registros[EBX] = 200;
    set_operando_reg(&vmx, OP1, TIPO_REGISTRO, EAX);
    set_operando_reg(&vmx, OP2, TIPO_REGISTRO, EBX);
    op_swap(&vmx);
    assert_registro(&vmx, EAX, 200, "SWAP intercambia EAX a 200");
    assert_registro(&vmx, EBX, 100, "SWAP intercambia EBX a 100");

    // 11. Test LDL y LDH: read-modify-write y no alteran CC
    vmx.registros[EAX] = 0x12345678;
    vmx.registros[CC] = 0x0;
    set_operando_reg(&vmx, OP1, TIPO_REGISTRO, EAX);
    set_operando_reg(&vmx, OP2, TIPO_INMEDIATO, 0xABCD);

    op_ldl(&vmx);
    assert_registro(&vmx, EAX, (int32_t)0x1234ABCD, "LDL carga 16 bits bajos preservando los altos");
    assert_registro(&vmx, CC, 0, "LDL no modifica el registro CC");

    op_ldh(&vmx);
    assert_registro(&vmx, EAX, (int32_t)0xABCDABCD, "LDH carga 16 bits altos preservando los bajos");
    assert_registro(&vmx, CC, 0, "LDH no modifica el registro CC");

    // 12. Test saltos: JMP incondicional y STOP
    vmx.registros[CS] = 0x00000000;
    vmx.registros[IP] = 0x00000000;
    set_operando_reg(&vmx, OP1, TIPO_INMEDIATO, 40);
    op_jmp(&vmx);
    assert_registro(&vmx, IP, 40, "JMP salta a offset 40");

    // Test JZ cuando Z=1 y cuando Z=0
    vmx.registros[CC] = (1 << 30); // Z=1
    set_operando_reg(&vmx, OP1, TIPO_INMEDIATO, 80);
    op_jz(&vmx);
    assert_registro(&vmx, IP, 80, "JZ salta cuando Z=1");

    vmx.registros[CC] = 0; // Z=0
    set_operando_reg(&vmx, OP1, TIPO_INMEDIATO, 120);
    op_jz(&vmx);
    assert_registro(&vmx, IP, 80, "JZ no salta cuando Z=0");

    // Test STOP
    op_stop(&vmx);
    assert_registro(&vmx, IP, -1, "STOP fija IP = -1");
}
