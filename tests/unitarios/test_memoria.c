#include "test_helpers.h"
#include "../../src/memoria.h"
#include "../../src/vmx.h"
#include <stdlib.h>

void test_memoria_suite(void) {
    printf("\n--- TESTS: memoria.c ---\n");

    Memoria mem;
    uint32_t regs[32];
    int i;

    for (i = 0; i < 32; i++) {
        regs[i] = 0;
    }

    // Test 1: inicializar_memoria
    inicializar_memoria(&mem, 16);
    mem.registros = regs;
    TEST_ASSERT_NOT_EQUAL((long long)mem.mem_principal, 0, "inicializar_memoria: buffer de memoria principal asignado");
    TEST_ASSERT_EQUAL(mem.tam_mem_kib, 16, "inicializar_memoria: tamano en KiB configurado en 16");
    TEST_ASSERT_EQUAL(mem.offset_entry_point, -1, "inicializar_memoria: entry point por defecto es -1");
    for (i = 0; i < TAM_SEGMENTOS; i++) {
        TEST_ASSERT_EQUAL(mem.tabla_segmentos[i], 0xFFFFFFFF, "inicializar_memoria: tabla_segmentos inicializada en 0xFFFFFFFF");
    }

    // Test 2: set_registros_memoria y proteccion contra punteros NULL
    set_registros_memoria(NULL, 0x00010004, 2, 104, 0x1234);
    TEST_ASSERT(1, "set_registros_memoria: seguro ante mem NULL");

    mem.registros = NULL;
    set_registros_memoria(&mem, 0x00010004, 2, 104, 0x1234);
    TEST_ASSERT(1, "set_registros_memoria: seguro ante registros NULL");

    mem.registros = regs;
    set_registros_memoria(&mem, 0x00010004, 2, 104, 0x1234);
    TEST_ASSERT_EQUAL(regs[LAR], 0x00010004, "set_registros_memoria: LAR almacena direccion logica");
    TEST_ASSERT_EQUAL(regs[MAR], (2 << 16) | 104, "set_registros_memoria: MAR contiene cant_bytes en alto y dir_fisica en bajo");
    TEST_ASSERT_EQUAL(regs[MBR], 0x1234, "set_registros_memoria: MBR contiene valor accedido");

    // Test 3: escribir_byte y casos limite (pos negativa y pos >= limite)
    escribir_byte(&mem, 0, 0xAA);
    escribir_byte(&mem, 16383, 0xBB);
    TEST_ASSERT_EQUAL(mem.mem_principal[0], 0xAA, "escribir_byte: escribe primer byte fisico 0");
    TEST_ASSERT_EQUAL(mem.mem_principal[16383], 0xBB, "escribir_byte: escribe ultimo byte fisico 16383");

    TEST_ASSERT_ABORTA(escribir_byte(&mem, -1, 0x55), "escribir_byte: posicion negativa aborta");
    TEST_ASSERT_ABORTA(escribir_byte(&mem, 16384, 0x55), "escribir_byte: posicion fuera de limite aborta");

    // Test 4: get_dir_fisica
    // Configuramos entrada 0: Base=100, Tamano=200
    mem.tabla_segmentos[0] = (100 << 16) | 200;
    int32_t dir_f = get_dir_fisica(&mem, 0x00000010); // Segmento 0, offset 16
    TEST_ASSERT_EQUAL(dir_f, 116, "get_dir_fisica: calcula correctamente base + offset");

    TEST_ASSERT_ABORTA(get_dir_fisica(&mem, 0x00010000), "get_dir_fisica: segmento 1 sin asignar (0xFFFFFFFF) aborta");
    TEST_ASSERT_ABORTA(get_dir_fisica(&mem, 0x00080000), "get_dir_fisica: codigo de segmento fuera de rango aborta");

    // Test 5: traducir_direccion con validaciones de limites
    uint16_t trad = traducir_direccion(&mem, 0x00000000, 4);
    TEST_ASSERT_EQUAL(trad, 100, "traducir_direccion: direccion base correcta");

    trad = traducir_direccion(&mem, 0x000000C4, 4); // offset 196 + 4 = 200 <= tamano 200
    TEST_ASSERT_EQUAL(trad, 296, "traducir_direccion: acceso al borde exacto del segmento");

    TEST_ASSERT_ABORTA(traducir_direccion(&mem, 0x000000C5, 4), "traducir_direccion: offset + cant > tam_segmento aborta");
    TEST_ASSERT_ABORTA(traducir_direccion(&mem, 0x00050000, 1), "traducir_direccion: segmento no asignado aborta");

    // Test 6: escribir_memoria y leer_memoria (1, 2, 3 y 4 bytes)
    escribir_memoria(&mem, 100, 1, 0x7A);
    TEST_ASSERT_EQUAL(leer_memoria(&mem, 100, 1), 0x7A, "escribir/leer_memoria: 1 byte");

    escribir_memoria(&mem, 102, 2, 0x1234);
    TEST_ASSERT_EQUAL(leer_memoria(&mem, 102, 2), 0x1234, "escribir/leer_memoria: 2 bytes big-endian");
    TEST_ASSERT_EQUAL(mem.mem_principal[102], 0x12, "escribir_memoria: byte mas significativo en direccion menor");
    TEST_ASSERT_EQUAL(mem.mem_principal[103], 0x34, "escribir_memoria: byte menos significativo en direccion mayor");

    escribir_memoria(&mem, 104, 3, 0x123456);
    TEST_ASSERT_EQUAL(leer_memoria(&mem, 104, 3), 0x123456, "escribir/leer_memoria: 3 bytes");

    escribir_memoria(&mem, 108, 4, 0x789ABCDE);
    TEST_ASSERT_EQUAL(leer_memoria(&mem, 108, 4), (int32_t)0x789ABCDE, "escribir/leer_memoria: 4 bytes");

    TEST_ASSERT_ABORTA(leer_memoria(&mem, 100, 0), "leer_memoria: cant_bytes 0 aborta");
    TEST_ASSERT_ABORTA(leer_memoria(&mem, 100, 5), "leer_memoria: cant_bytes 5 aborta");
    TEST_ASSERT_ABORTA(escribir_memoria(&mem, 100, 0, 1), "escribir_memoria: cant_bytes 0 aborta");
    TEST_ASSERT_ABORTA(escribir_memoria(&mem, 100, 5, 1), "escribir_memoria: cant_bytes 5 aborta");

    // Test 7: Acceso fisico en el limite superior exacto de memoria principal
    // Memoria de 16 KiB = 16384 bytes. Las direcciones van de 0 a 16383.
    // 4 bytes en 16380 (16380, 16381, 16382, 16383) es el limite exacto.
    escribir_memoria(&mem, 16380, 4, 0x55AA33CC);
    TEST_ASSERT_EQUAL((uint32_t)leer_memoria(&mem, 16380, 4), (uint32_t)0x55AA33CC, "escribir/leer_memoria: limite superior exacto de RAM (16380..16383)");

    // Intentar leer 4 bytes en 16381 excede por 1 byte -> aborta
    TEST_ASSERT_ABORTA(leer_memoria(&mem, 16381, 4), "leer_memoria: desborde en limite superior fisico aborta");

    // Test 8: get_valor_memoria y set_valor_memoria con extension de signo
    // Escribimos un byte negativo (0xFE = -2)
    set_valor_memoria(&mem, 0x0000000A, 0xFE, 1);
    int32_t val1 = get_valor_memoria(&mem, 0x0000000A, 1);
    TEST_ASSERT_EQUAL(val1, -2, "get_valor_memoria: propaga signo en lectura de 1 byte (0xFE -> -2)");

    // Escribimos dos bytes negativos (0xFF80 = -128)
    set_valor_memoria(&mem, 0x0000000C, 0xFF80, 2);
    int32_t val2 = get_valor_memoria(&mem, 0x0000000C, 2);
    TEST_ASSERT_EQUAL(val2, -128, "get_valor_memoria: propaga signo en lectura de 2 bytes (0xFF80 -> -128)");

    TEST_ASSERT_EQUAL(regs[LAR], 0x0000000C, "get_valor_memoria: actualiza LAR");
    TEST_ASSERT_EQUAL(regs[MBR], -128, "get_valor_memoria: actualiza MBR con valor leido");

    TEST_ASSERT_ABORTA(get_valor_memoria(&mem, 0x00000000, 0), "get_valor_memoria: cant_bytes < 1 aborta");
    TEST_ASSERT_ABORTA(get_valor_memoria(&mem, 0x00000000, 5), "get_valor_memoria: cant_bytes > 4 aborta");
    TEST_ASSERT_ABORTA(set_valor_memoria(&mem, 0x00000000, 1, 0), "set_valor_memoria: cant_bytes < 1 aborta");
    TEST_ASSERT_ABORTA(set_valor_memoria(&mem, 0x00000000, 1, 5), "set_valor_memoria: cant_bytes > 4 aborta");

    // Test 9: crear_segmentos con todos los segmentos presentes
    // Creamos: par=10, const=20, code=30, data=0, extra=40, stack=50
    crear_segmentos(&mem, 10, 20, 30, 0, 40, 50);
    // Entrada 0: Param (base 0, tam 10)
    TEST_ASSERT_EQUAL(mem.tabla_segmentos[0], (0 << 16) | 10, "crear_segmentos: PS en entrada 0 base 0 tam 10");
    TEST_ASSERT_EQUAL(regs[PS], 0x00000000, "crear_segmentos: reg PS apunta a entrada 0");

    // Entrada 1: Const (base 10, tam 20)
    TEST_ASSERT_EQUAL(mem.tabla_segmentos[1], (10 << 16) | 20, "crear_segmentos: KS en entrada 1 base 10 tam 20");
    TEST_ASSERT_EQUAL(regs[KS], 0x00010000, "crear_segmentos: reg KS apunta a entrada 1");

    // Entrada 2: Code (base 30, tam 30)
    TEST_ASSERT_EQUAL(mem.tabla_segmentos[2], (30 << 16) | 30, "crear_segmentos: CS en entrada 2 base 30 tam 30");
    TEST_ASSERT_EQUAL(regs[CS], 0x00020000, "crear_segmentos: reg CS apunta a entrada 2");

    // DS = 0 (ausente) -> reg DS debe ser -1
    TEST_ASSERT_EQUAL(regs[DS], -1, "crear_segmentos: DS ausente queda en -1");

    // Entrada 3: Extra (base 60, tam 40)
    TEST_ASSERT_EQUAL(mem.tabla_segmentos[3], (60 << 16) | 40, "crear_segmentos: ES en entrada 3 base 60 tam 40");
    TEST_ASSERT_EQUAL(regs[ES], 0x00030000, "crear_segmentos: reg ES apunta a entrada 3");

    // Entrada 4: Stack (base 100, tam 50)
    TEST_ASSERT_EQUAL(mem.tabla_segmentos[4], (100 << 16) | 50, "crear_segmentos: SS en entrada 4 base 100 tam 50");
    TEST_ASSERT_EQUAL(regs[SS], 0x00040000, "crear_segmentos: reg SS apunta a entrada 4");

    // Entrada 5..7 deben quedar sin asignar (0xFFFFFFFF)
    TEST_ASSERT_EQUAL(mem.tabla_segmentos[5], 0xFFFFFFFF, "crear_segmentos: entrada 5 libre");
    TEST_ASSERT_EQUAL(mem.tabla_segmentos[6], 0xFFFFFFFF, "crear_segmentos: entrada 6 libre");
    TEST_ASSERT_EQUAL(mem.tabla_segmentos[7], 0xFFFFFFFF, "crear_segmentos: entrada 7 libre");

    // SP debe inicializarse en SS + tam_stack
    TEST_ASSERT_EQUAL(regs[SP], (4 << 16) | 50, "crear_segmentos: SP inicializado en SS + tam_stack");

    // Test 10: crear_segmentos con solo CS y SS (minimo legal)
    crear_segmentos(&mem, 0, 0, 80, 0, 0, 120);
    TEST_ASSERT_EQUAL(regs[PS], -1, "crear_segmentos minimo: PS ausente es -1");
    TEST_ASSERT_EQUAL(regs[KS], -1, "crear_segmentos minimo: KS ausente es -1");
    TEST_ASSERT_EQUAL(regs[DS], -1, "crear_segmentos minimo: DS ausente es -1");
    TEST_ASSERT_EQUAL(regs[ES], -1, "crear_segmentos minimo: ES ausente es -1");
    TEST_ASSERT_EQUAL(regs[CS], 0x00000000, "crear_segmentos minimo: CS en entrada 0");
    TEST_ASSERT_EQUAL(mem.tabla_segmentos[0], (0 << 16) | 80, "crear_segmentos minimo: CS base 0 tam 80");
    TEST_ASSERT_EQUAL(regs[SS], 0x00010000, "crear_segmentos minimo: SS en entrada 1");
    TEST_ASSERT_EQUAL(mem.tabla_segmentos[1], (80 << 16) | 120, "crear_segmentos minimo: SS base 80 tam 120");
    TEST_ASSERT_EQUAL(regs[SP], (1 << 16) | 120, "crear_segmentos minimo: SP en SS + 120");
    TEST_ASSERT_EQUAL(mem.tabla_segmentos[2], 0xFFFFFFFF, "crear_segmentos minimo: entrada 2 libre");

    // Test 11: crear_segmentos con memoria insuficiente
    // Configuramos memoria principal en 1 KiB (1024 bytes)
    mem.tam_mem_kib = 1;
    // Intentamos cargar segmentos que suman 1025 bytes (CS=600, SS=425 -> 1025 > 1024)
    TEST_ASSERT_ABORTA(crear_segmentos(&mem, 0, 0, 600, 0, 0, 425), "crear_segmentos: desborde de RAM aborta con memoria insuficiente");

    free(mem.mem_principal);
}
