#include "tests_helpers.h"
#include "../../src/operandos.h"
#include "../../src/operadores.h"
#include "../../src/memoria.h"
#include "../../src/errores.h"
#include <stdio.h>
#include <setjmp.h>

static jmp_buf s_jmp_abort_operandos;
static int s_abort_llamado_operandos = 0;

static void mock_abortar_operandos(char mensaje[]) {
    (void)mensaje;
    s_abort_llamado_operandos = 1;
    longjmp(s_jmp_abort_operandos, 1);
}

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

    // 4. Tests de memoria y registros de bus LAR, MAR, MBR
    // Configuramos segmento 1 (datos): base = 100, tamano = 16284
    vmx.memoria.tabla_segmentos[0] = 100;
    vmx.memoria.tabla_segmentos[1] = (100 << 16) | 16284;
    vmx.registros[DS] = 0x00010000;

    // Operando de memoria [DS + 10]: offset 10, registro DS (27)
    // Dato empaquetado: (10 << 8) | DS
    int32_t op_mem = (10 << 8) | DS;
    set_valor(&vmx, TIPO_MEMORIA, op_mem, 0x11223344);

    // Verifica que se escribio correctamente y actualizo registros de bus
    ASSERT_EQUAL(vmx.registros[LAR], 0x0001000A, "set_valor (memoria): LAR actualizado a 0x0001000A");
    ASSERT_EQUAL(vmx.registros[MBR], 0x11223344, "set_valor (memoria): MBR actualizado con valor escrito");
    ASSERT_EQUAL((int)(vmx.registros[MAR] & 0xFFFF), 110, "set_valor (memoria): MAR direccion fisica = 110 (100 + 10)");
    ASSERT_EQUAL((int)((vmx.registros[MAR] >> 16) & 0xFFFF), 4, "set_valor (memoria): MAR cant_bytes = 4");

    // Lee de memoria mediante get_valor
    int32_t val_mem = get_valor(&vmx, TIPO_MEMORIA, op_mem);
    ASSERT_EQUAL(val_mem, 0x11223344, "get_valor (memoria): Lee 0x11223344 desde [DS+10]");
    ASSERT_EQUAL(vmx.registros[MBR], 0x11223344, "get_valor (memoria): MBR actualizado con valor leido");

    // Test de las funciones con struct Memoria
    Memoria mem_test;
    inicializar_memoria(&mem_test);
    mem_test.tabla_segmentos[1] = (100 << 16) | 1000;
    set_valor_memoria(&mem_test, 0x00010006, 0x59B, 3);
    int32_t val_mem_test = get_valor_memoria(&mem_test, 0x00010006, 3);
    ASSERT_EQUAL(val_mem_test, 0x59B, "get_valor_memoria/set_valor_memoria: Lee 0x59B de direccion logica");

    // 5. Tests combinar_mitad (read-modify-write para LDH y LDL)
    // Inicializa EAX con 0x12345678
    vmx.registros[EAX] = 0x12345678;

    // LDH: cambia parte alta por 0xAAAA -> debe quedar 0xAAAA5678
    combinar_mitad(&vmx, TIPO_REGISTRO, EAX, 0xAAAA, 1);
    ASSERT_EQUAL(vmx.registros[EAX], (int32_t)0xAAAA5678, "combinar_mitad (LDH): Modifica parte alta a 0xAAAA y preserva parte baja");

    // LDL: cambia parte baja por 0xBBBB -> debe quedar 0xAAAABBBB
    combinar_mitad(&vmx, TIPO_REGISTRO, EAX, 0xBBBB, 0);
    ASSERT_EQUAL(vmx.registros[EAX], (int32_t)0xAAAABBBB, "combinar_mitad (LDL): Modifica parte baja a 0xBBBB y preserva parte alta");

    // combinar_mitad en memoria: [DS + 10] tenia 0x11223344
    combinar_mitad(&vmx, TIPO_MEMORIA, op_mem, 0x9999, 1); // Parte alta -> 0x99993344
    ASSERT_EQUAL(get_valor(&vmx, TIPO_MEMORIA, op_mem), (int32_t)0x99993344, "combinar_mitad (LDH memoria): Modifica parte alta en memoria");

    // 6. Tests actualizar_cc (casos exactos de la tabla de la especificacion)
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

    // 7. Pruebas de aborto con setjmp ante accesos invalidos
    set_abortar_handler(mock_abortar_operandos);

    // Intento de direccionamiento de memoria con offset negativo que desborda el segmento ([DS - 4])
    s_abort_llamado_operandos = 0;
    if (setjmp(s_jmp_abort_operandos) == 0) {
        // Offset -4 crudo en 16 bits = 0xFFFC
        calcular_direccion_logica_memoria(&vmx, ((int32_t)0xFFFC << 8) | DS);
    }
    ASSERT(s_abort_llamado_operandos == 1, "calcular_direccion_logica_memoria: Desplazamiento negativo fuera de segmento aborta");

    // Intento de acceder fuera de los limites del segmento aborta en traducir_direccion
    s_abort_llamado_operandos = 0;
    if (setjmp(s_jmp_abort_operandos) == 0) {
        traducir_direccion(&vmx.memoria, 0x00010000 + 20000, 4);
    }
    ASSERT(s_abort_llamado_operandos == 1, "traducir_direccion: Acceso fuera de los limites del segmento aborta");

    // Intento de escribir en operando inmediato
    s_abort_llamado_operandos = 0;
    if (setjmp(s_jmp_abort_operandos) == 0) {
        set_valor(&vmx, TIPO_INMEDIATO, 10, 50);
    }
    ASSERT(s_abort_llamado_operandos == 1, "set_valor: Intento de escribir en operando inmediato aborta");

    set_abortar_handler(NULL);
}
