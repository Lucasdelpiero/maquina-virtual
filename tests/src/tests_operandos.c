#include "tests_helpers.h"
#include "../../src/operandos.h"
#include "../../src/operadores.h"
#include <stdio.h>

void correr_tests_operandos(void) {
    Vmx vmx;
    int32_t op_empaquetado;
    int32_t val_extendido;

    printf("\n--- TESTS: Modulo Operandos y Codigo de Condicion ---\n");

    inicializar_vmx(&vmx, 0, 0);

    // 1. Tests get_tipo y get_dato
    // Empaqueta: Tipo 1 (registro) en byte alto, dato 10 (EAX) en 24 bits bajos
    op_empaquetado = (1 << 24) | 10;
    ASSERT_EQUAL(get_tipo(op_empaquetado), TIPO_REGISTRO, "get_tipo: Extrae tipo registro (1)");
    ASSERT_EQUAL(get_dato(op_empaquetado), 10, "get_dato: Extrae dato registro (10)");

    // Empaqueta: Tipo 2 (inmediato) con dato 0x0000002A (42)
    op_empaquetado = (2 << 24) | 42;
    ASSERT_EQUAL(get_tipo(op_empaquetado), TIPO_INMEDIATO, "get_tipo: Extrae tipo inmediato (2)");
    ASSERT_EQUAL(get_dato(op_empaquetado), 42, "get_dato: Extrae dato inmediato (42)");

    // 2. Tests extender_signo_16_a_32
    // Positivo: 5 -> 5
    val_extendido = extender_signo_16_a_32(5);
    ASSERT_EQUAL(val_extendido, 5, "extender_signo: Positivo 5 se mantiene en 5");

    // Negativo: 0xFFFF (-1 en 16 bits) -> -1 en 32 bits (0xFFFFFFFF)
    val_extendido = extender_signo_16_a_32(0xFFFF);
    ASSERT_EQUAL(val_extendido, -1, "extender_signo: 0xFFFF se extiende correctamente a -1");

    // Negativo limite: 0x8000 (-32768 en 16 bits) -> -32768 en 32 bits
    val_extendido = extender_signo_16_a_32(0x8000);
    ASSERT_EQUAL(val_extendido, -32768, "extender_signo: 0x8000 se extiende correctamente a -32768");

    // 3. Tests get_valor y set_valor con registros
    set_valor(&vmx, TIPO_REGISTRO, EAX, 12345);
    ASSERT_EQUAL(vmx.registros[EAX], 12345, "set_valor: Escribe 12345 en registro EAX");
    ASSERT_EQUAL(get_valor(&vmx, TIPO_REGISTRO, EAX), 12345, "get_valor: Lee 12345 de registro EAX");

    // Test get_valor con inmediato negativo
    // Inmediato -50: 0xFFCE en 16 bits
    ASSERT_EQUAL(get_valor(&vmx, TIPO_INMEDIATO, 0xFFCE), -50, "get_valor: Inmediato negativo 0xFFCE se lee como -50");

    // 4. Tests combinar_mitad (read-modify-write para LDH y LDL)
    // Inicializa EAX con 0x12345678
    vmx.registros[EAX] = 0x12345678;

    // LDH: cambia parte alta por 0xAAAA -> debe quedar 0xAAAA5678
    combinar_mitad(&vmx, TIPO_REGISTRO, EAX, 0xAAAA, 1);
    ASSERT_EQUAL(vmx.registros[EAX], (int32_t)0xAAAA5678, "combinar_mitad (LDH): Modifica parte alta a 0xAAAA y preserva parte baja");

    // LDL: cambia parte baja por 0xBBBB -> debe quedar 0xAAAABBBB
    combinar_mitad(&vmx, TIPO_REGISTRO, EAX, 0xBBBB, 0);
    ASSERT_EQUAL(vmx.registros[EAX], (int32_t)0xAAAABBBB, "combinar_mitad (LDL): Modifica parte baja a 0xBBBB y preserva parte alta");

    // 5. Tests actualizar_cc (casos exactos de la tabla de la especificacion)
    // Caso 1: MOV -> res = -1, C=0, V=0 -> N=1, Z=0, C=0, V=0
    actualizar_cc(&vmx, -1, 0, 0);
    assert_cc(&vmx, 1, 0, 0, 0, "actualizar_cc: Caso MOV (res = -1)");

    // Caso 2: ADD 2147483647, 2147483647 -> res = -2, C=0, V=1 -> N=1, Z=0, C=0, V=1
    actualizar_cc(&vmx, -2, 0, 1);
    assert_cc(&vmx, 1, 0, 0, 1, "actualizar_cc: Caso ADD (res = -2, V=1)");

    // Caso 3: SUB -3, 5 -> res = -8, C=1, V=0 -> N=1, Z=0, C=1, V=0
    actualizar_cc(&vmx, -8, 1, 0);
    assert_cc(&vmx, 1, 0, 1, 0, "actualizar_cc: Caso SUB (res = -8, C=1)");

    // Caso 4: MUL 1073741827, 4 -> res = 12, C=1, V=1 -> N=0, Z=0, C=1, V=1
    actualizar_cc(&vmx, 12, 1, 1);
    assert_cc(&vmx, 0, 0, 1, 1, "actualizar_cc: Caso MUL (res = 12, C=1, V=1)");

    // Caso 5: DIV 6, 7 -> res = 0, C=0, V=0 -> N=0, Z=1, C=0, V=0
    actualizar_cc(&vmx, 0, 0, 0);
    assert_cc(&vmx, 0, 1, 0, 0, "actualizar_cc: Caso DIV (res = 0, Z=1)");

    // Caso 6: SHL 1073741824, 2 -> res = 0, C=1, V=1 -> N=0, Z=1, C=1, V=1
    actualizar_cc(&vmx, 0, 1, 1);
    assert_cc(&vmx, 0, 1, 1, 1, "actualizar_cc: Caso SHL (res = 0, Z=1, C=1, V=1)");
}
