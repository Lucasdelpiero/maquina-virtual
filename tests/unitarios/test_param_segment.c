#include "test_helpers.h"
#include "../../src/param_segment.h"
#include "../../src/inicializador.h"

void test_param_segment_suite(void) {
    printf("\n--- TESTS: param_segment.c ---\n");

    Vmx vmx;
    inicializar_vmx(&vmx, 16, 0, 0);

    // Test 1: Sin parametros (nro_params = 0)
    char params[MAX_PARAMS][MAX_PARAM_LEN];
    InfoParamSegment info0 = construir_param_segment(&vmx, "programa.vmx", params, 0);
    TEST_ASSERT_EQUAL(info0.existe, 0, "construir_param_segment: sin params existe es 0");
    TEST_ASSERT_EQUAL(info0.tamano, 0, "construir_param_segment: tamano es 0");
    TEST_ASSERT_EQUAL(info0.argc, 0, "construir_param_segment: argc es 0");
    TEST_ASSERT_EQUAL(info0.dir_argv, 0xFFFFFFFF, "construir_param_segment: dir_argv es 0xFFFFFFFF");
    TEST_ASSERT_EQUAL(vmx.registros[PS], -1, "construir_param_segment: reg PS queda en -1");

    // Test 2: Con parametros negativos (nro_params = -1)
    InfoParamSegment info_neg = construir_param_segment(&vmx, "prog.vmx", params, -1);
    TEST_ASSERT_EQUAL(info_neg.existe, 0, "construir_param_segment: nro_params < 0 tratado como sin params");

    // Test 3: Con 1 solo parametro ("prog.vmx -p simple")
    strcpy(params[0], "simple");
    InfoParamSegment info1 = construir_param_segment(&vmx, "prog.vmx", params, 1);
    TEST_ASSERT_EQUAL(info1.existe, 1, "construir_param_segment 1 param: existe es 1");
    TEST_ASSERT_EQUAL(info1.argc, 2, "construir_param_segment 1 param: argc es 2");
    // "prog.vmx\0" (9 bytes) + "simple\0" (7 bytes) + 2*4 bytes = 24 bytes
    TEST_ASSERT_EQUAL(info1.tamano, 24, "construir_param_segment 1 param: tamano es 24");
    TEST_ASSERT_EQUAL(info1.dir_argv, 16, "construir_param_segment 1 param: dir_argv en offset 16");
    TEST_ASSERT_STR_EQUAL((char*)&vmx.memoria.mem_principal[0], "prog.vmx", "Param 0: prog.vmx");
    TEST_ASSERT_STR_EQUAL((char*)&vmx.memoria.mem_principal[9], "simple", "Param 1: simple");

    // Test 4: Con parametro vacio ("")
    strcpy(params[0], "");
    InfoParamSegment info_vacio = construir_param_segment(&vmx, "prog.vmx", params, 1);
    TEST_ASSERT_EQUAL(info_vacio.existe, 1, "construir_param_segment param vacio: existe es 1");
    // "prog.vmx\0" (9) + "\0" (1) + 2*4 (8) = 18 bytes
    TEST_ASSERT_EQUAL(info_vacio.tamano, 18, "construir_param_segment param vacio: tamano es 18 bytes");

    // Test 5: Con parametros del ejemplo de la especificacion ("prog.vmx -p prueba 01 2")
    strcpy(params[0], "prueba");
    strcpy(params[1], "01");
    strcpy(params[2], "2");
    InfoParamSegment info = construir_param_segment(&vmx, "prog.vmx", params, 3);

    TEST_ASSERT_EQUAL(info.existe, 1, "construir_param_segment: existe es 1");
    TEST_ASSERT_EQUAL(info.argc, 4, "construir_param_segment: argc es 4 (nombre + 3 params)");
    TEST_ASSERT_EQUAL(info.tamano, 37, "construir_param_segment: tamano exacto 37 bytes segun especificacion");
    TEST_ASSERT_EQUAL(info.dir_argv, 21, "construir_param_segment: argv inicia en offset 21");
    TEST_ASSERT_EQUAL(vmx.registros[PS], 0x00000000, "construir_param_segment: PS apunta a 0x00000000");
    TEST_ASSERT_EQUAL(vmx.memoria.tabla_segmentos[0], 37, "construir_param_segment: entrada 0 de tabla tiene tamano 37");

    // Verificamos los strings escritos en memoria fisica
    TEST_ASSERT_STR_EQUAL((char*)&vmx.memoria.mem_principal[0], "prog.vmx", "Param 0: prog.vmx en offset 0");
    TEST_ASSERT_STR_EQUAL((char*)&vmx.memoria.mem_principal[9], "prueba", "Param 1: prueba en offset 9");
    TEST_ASSERT_STR_EQUAL((char*)&vmx.memoria.mem_principal[16], "01", "Param 2: 01 en offset 16");
    TEST_ASSERT_STR_EQUAL((char*)&vmx.memoria.mem_principal[19], "2", "Param 3: 2 en offset 19");

    // Verificamos los punteros de argv (4 punteros de 4 bytes)
    uint32_t p0 = ((uint32_t)vmx.memoria.mem_principal[21] << 24) |
                  ((uint32_t)vmx.memoria.mem_principal[22] << 16) |
                  ((uint32_t)vmx.memoria.mem_principal[23] << 8) |
                  (uint32_t)vmx.memoria.mem_principal[24];
    TEST_ASSERT_EQUAL(p0, 0, "argv[0] apunta a offset 0");

    uint32_t p1 = ((uint32_t)vmx.memoria.mem_principal[25] << 24) |
                  ((uint32_t)vmx.memoria.mem_principal[26] << 16) |
                  ((uint32_t)vmx.memoria.mem_principal[27] << 8) |
                  (uint32_t)vmx.memoria.mem_principal[28];
    TEST_ASSERT_EQUAL(p1, 9, "argv[1] apunta a offset 9");

    uint32_t p2 = ((uint32_t)vmx.memoria.mem_principal[29] << 24) |
                  ((uint32_t)vmx.memoria.mem_principal[30] << 16) |
                  ((uint32_t)vmx.memoria.mem_principal[31] << 8) |
                  (uint32_t)vmx.memoria.mem_principal[32];
    TEST_ASSERT_EQUAL(p2, 16, "argv[2] apunta a offset 16");

    uint32_t p3 = ((uint32_t)vmx.memoria.mem_principal[33] << 24) |
                  ((uint32_t)vmx.memoria.mem_principal[34] << 16) |
                  ((uint32_t)vmx.memoria.mem_principal[35] << 8) |
                  (uint32_t)vmx.memoria.mem_principal[36];
    TEST_ASSERT_EQUAL(p3, 19, "argv[3] apunta a offset 19");

    // Test 6: Memoria insuficiente
    vmx.memoria.tam_mem_kib = 0;
    TEST_ASSERT_ABORTA(construir_param_segment(&vmx, "prog.vmx", params, 3), "construir_param_segment: memoria insuficiente aborta");

    free(vmx.memoria.mem_principal);
}
