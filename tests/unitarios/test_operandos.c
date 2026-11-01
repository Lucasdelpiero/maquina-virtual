#include "test_helpers.h"
#include "../../src/operandos.h"
#include "../../src/memoria.h"
#include "../../src/inicializador.h"

void test_operandos_suite(void) {
    printf("\n--- TESTS: operandos.c ---\n");

    Vmx vmx;
    inicializar_vmx(&vmx, 16, 0, 0);

    // Test 1: get_tipo y get_dato
    int32_t op_reg = (TIPO_REGISTRO << 24) | 0x001234;
    int32_t op_inm = (TIPO_INMEDIATO << 24) | 0x00FF80;
    int32_t op_mem = (TIPO_MEMORIA << 24) | 0x00AABB;
    int32_t op_nin = (TIPO_NINGUNO << 24) | 0x000000;

    TEST_ASSERT_EQUAL(get_tipo(op_reg), TIPO_REGISTRO, "get_tipo: extrae tipo REGISTRO (1)");
    TEST_ASSERT_EQUAL(get_tipo(op_inm), TIPO_INMEDIATO, "get_tipo: extrae tipo INMEDIATO (2)");
    TEST_ASSERT_EQUAL(get_tipo(op_mem), TIPO_MEMORIA, "get_tipo: extrae tipo MEMORIA (3)");
    TEST_ASSERT_EQUAL(get_tipo(op_nin), TIPO_NINGUNO, "get_tipo: extrae tipo NINGUNO (0)");

    TEST_ASSERT_EQUAL(get_dato(op_reg), 0x001234, "get_dato: extrae 24 bits inferiores");
    TEST_ASSERT_EQUAL(get_dato(op_mem), 0x00AABB, "get_dato: extrae dato sin alterar bits");

    // Test 2: extender_signo_16_a_32 (casos borde y valores 2 -> 4)
    TEST_ASSERT_EQUAL(extender_signo_16_a_32(0x0000), 0, "extender_signo_16_a_32: 0x0000 es 0");
    TEST_ASSERT_EQUAL(extender_signo_16_a_32(0x0002), 2, "extender_signo_16_a_32: 0x0002 es 2");
    TEST_ASSERT_EQUAL(extender_signo_16_a_32(0x0004), 4, "extender_signo_16_a_32: 0x0004 es 4");
    TEST_ASSERT_EQUAL(extender_signo_16_a_32(0x7FFF), 32767, "extender_signo_16_a_32: 0x7FFF es 32767");
    TEST_ASSERT_EQUAL(extender_signo_16_a_32(0x8000), -32768, "extender_signo_16_a_32: 0x8000 es -32768");
    TEST_ASSERT_EQUAL(extender_signo_16_a_32(0xFFFF), -1, "extender_signo_16_a_32: 0xFFFF es -1");
    TEST_ASSERT_EQUAL(extender_signo_16_a_32(0xFFFE), -2, "extender_signo_16_a_32: 0xFFFE es -2");

    // Test 3: get_cantidad_bytes_accedidos
    // bits 7..5: 001 (1b), 010 (2b), 100 (4b)
    TEST_ASSERT_EQUAL(get_cantidad_bytes_accedidos(1 << 5), 1, "get_cantidad_bytes_accedidos: 001 es 1 byte");
    TEST_ASSERT_EQUAL(get_cantidad_bytes_accedidos(2 << 5), 2, "get_cantidad_bytes_accedidos: 010 es 2 bytes");
    TEST_ASSERT_EQUAL(get_cantidad_bytes_accedidos(4 << 5), 4, "get_cantidad_bytes_accedidos: 100 es 4 bytes");
    TEST_ASSERT_EQUAL(get_cantidad_bytes_accedidos(0), 4, "get_cantidad_bytes_accedidos: 000 por defecto es 4 bytes");

    // Test 4: Sectores de registro en EAX, EBX, ECX, EDX
    // EAX: 0x11223344
    vmx.registros[EAX] = 0x11223344;
    TEST_ASSERT_EQUAL(get_valor(&vmx, TIPO_REGISTRO, (0 << 5) | EAX), (int32_t)0x11223344, "get_valor: EAX completo");
    TEST_ASSERT_EQUAL(get_valor(&vmx, TIPO_REGISTRO, (1 << 5) | EAX), 0x44, "get_valor: AL devuelve byte 0 (0x44)");
    TEST_ASSERT_EQUAL(get_valor(&vmx, TIPO_REGISTRO, (2 << 5) | EAX), 0x33, "get_valor: AH devuelve byte 1 (0x33)");
    TEST_ASSERT_EQUAL(get_valor(&vmx, TIPO_REGISTRO, (3 << 5) | EAX), 0x3344, "get_valor: AX devuelve 16 bits bajos");

    // EDX: probar DL, DH, DX
    vmx.registros[EDX] = 0xAABBCCDD;
    TEST_ASSERT_EQUAL(get_valor(&vmx, TIPO_REGISTRO, (1 << 5) | EDX), -35, "get_valor: DL (0xDD con signo = -35)");
    TEST_ASSERT_EQUAL(get_valor(&vmx, TIPO_REGISTRO, (2 << 5) | EDX), -52, "get_valor: DH (0xCC con signo = -52)");
    TEST_ASSERT_EQUAL(get_valor(&vmx, TIPO_REGISTRO, (3 << 5) | EDX), -13091, "get_valor: DX (0xCCDD con signo = -13091)");

    // Escritura en sectores sin alterar el resto del registro
    vmx.registros[ECX] = 0x12345678;
    set_valor(&vmx, TIPO_REGISTRO, (1 << 5) | ECX, 0xAA); // Modifica solo CL
    TEST_ASSERT_EQUAL((uint32_t)vmx.registros[ECX], (uint32_t)0x123456AA, "set_valor: modifica solo CL manteniendo resto");

    set_valor(&vmx, TIPO_REGISTRO, (2 << 5) | ECX, 0xBB); // Modifica solo CH
    TEST_ASSERT_EQUAL((uint32_t)vmx.registros[ECX], (uint32_t)0x1234BBAA, "set_valor: modifica solo CH manteniendo resto");

    set_valor(&vmx, TIPO_REGISTRO, (3 << 5) | ECX, 0xCCDD); // Modifica solo CX
    TEST_ASSERT_EQUAL((uint32_t)vmx.registros[ECX], (uint32_t)0x1234CCDD, "set_valor: modifica solo CX manteniendo parte alta");

    // Test 5: Direcciones de memoria y calcular_direccion_logica_memoria
    // Asignamos segmento 0: base 0, tamano 500
    vmx.memoria.tabla_segmentos[0] = (0 << 16) | 500;
    vmx.registros[DS] = 0x00000000;

    // Offset positivo 10 con registro DS (cod 27)
    int32_t dato_mem_pos = (10 << 8) | DS;
    int32_t dir_log_pos = calcular_direccion_logica_memoria(&vmx, dato_mem_pos);
    TEST_ASSERT_EQUAL(dir_log_pos, 0x0000000A, "calcular_direccion_logica_memoria: offset positivo");

    // Offset negativo relativo legal (32 - 10 = 22)
    vmx.registros[EDX] = 0x00000020; // offset 32
    int32_t dato_mem_neg = (((uint16_t)-10) << 8) | EDX; // offset -10
    int32_t dir_log_neg = calcular_direccion_logica_memoria(&vmx, dato_mem_neg);
    TEST_ASSERT_EQUAL(dir_log_neg, 0x00000016, "calcular_direccion_logica_memoria: offset negativo relativo (32 - 10 = 22)");

    // Offset negativo que da resultado menor a cero aborta
    vmx.registros[EDX] = 0x00000005; // offset 5
    int32_t dato_mem_under = (((uint16_t)-10) << 8) | EDX; // offset -10 -> 5 - 10 = -5
    TEST_ASSERT_ABORTA(calcular_direccion_logica_memoria(&vmx, dato_mem_under), "calcular_direccion_logica_memoria: offset total < 0 aborta");

    // resolver_direccion_memoria
    uint16_t dir_fis = resolver_direccion_memoria(&vmx, dato_mem_pos);
    TEST_ASSERT_EQUAL(dir_fis, 10, "resolver_direccion_memoria: traduce logica a fisica");

    // Test 6: get_valor y set_valor en memoria con modificadores y extension de signo
    int32_t dato_mem_byte = (1 << 5) | (20 << 8) | DS; // b[DS+20]
    set_valor(&vmx, TIPO_MEMORIA, dato_mem_byte, 0x80); // 0x80 con signo es -128
    TEST_ASSERT_EQUAL(get_valor(&vmx, TIPO_MEMORIA, dato_mem_byte), -128, "get/set_valor memoria byte: 0x80 extiende signo a -128");

    int32_t dato_mem_word = (2 << 5) | (22 << 8) | DS; // w[DS+22]
    set_valor(&vmx, TIPO_MEMORIA, dato_mem_word, 0x8000); // 0x8000 con signo es -32768
    TEST_ASSERT_EQUAL(get_valor(&vmx, TIPO_MEMORIA, dato_mem_word), -32768, "get/set_valor memoria word: 0x8000 extiende signo a -32768");

    int32_t dato_mem_long = (4 << 5) | (24 << 8) | DS; // l[DS+24]
    set_valor(&vmx, TIPO_MEMORIA, dato_mem_long, (int32_t)0x80000000);
    TEST_ASSERT_EQUAL(get_valor(&vmx, TIPO_MEMORIA, dato_mem_long), (int32_t)0x80000000, "get/set_valor memoria long: 0x80000000 es INT32_MIN");

    // Test 7: Error al escribir en inmediato o tipo desconocido
    TEST_ASSERT_ABORTA(set_valor(&vmx, TIPO_INMEDIATO, 10, 20), "set_valor: escribir en inmediato aborta");
    TEST_ASSERT_ABORTA(set_valor(&vmx, 99, 0, 0), "set_valor: tipo invalido aborta");
    TEST_ASSERT_ABORTA(get_valor(&vmx, 99, 0), "get_valor: tipo invalido aborta");

    // Test 8: combinar_mitad (LDH y LDL)
    vmx.registros[EAX] = 0x11223344;
    combinar_mitad(&vmx, TIPO_REGISTRO, EAX, 0xAAAA, 1); // LDH alta
    TEST_ASSERT_EQUAL((uint32_t)vmx.registros[EAX], (uint32_t)0xAAAA3344, "combinar_mitad: LDH actualiza 16 bits altos");

    combinar_mitad(&vmx, TIPO_REGISTRO, EAX, 0xBBBB, 0); // LDL baja
    TEST_ASSERT_EQUAL((uint32_t)vmx.registros[EAX], (uint32_t)0xAAAABBBB, "combinar_mitad: LDL actualiza 16 bits bajos");

    // combinar_mitad sobre memoria
    set_valor(&vmx, TIPO_MEMORIA, dato_mem_long, 0x11112222);
    combinar_mitad(&vmx, TIPO_MEMORIA, dato_mem_long, 0x9999, 1); // LDH en memoria
    TEST_ASSERT_EQUAL((uint32_t)get_valor(&vmx, TIPO_MEMORIA, dato_mem_long), (uint32_t)0x99992222, "combinar_mitad: LDH en memoria actualiza alta");

    free(vmx.memoria.mem_principal);
}
