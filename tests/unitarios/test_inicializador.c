#include "test_helpers.h"
#include "../../src/inicializador.h"
#include "../../src/memoria.h"

void test_inicializador_suite(void) {
    printf("\n--- TESTS: inicializador.c ---\n");

    // Test 1: setear_tabla_segmento
    Memoria mem;
    inicializar_memoria(&mem, 16); // 16 KiB = 16384 bytes
    setear_tabla_segmento(&mem, 500);

    // Entrada 0 (CS): Base 0, Tamano 500
    TEST_ASSERT_EQUAL(mem.tabla_segmentos[0], 500, "setear_tabla_segmento: entrada 0 base 0 tam 500");
    // Entrada 1 (DS): Base 500, Tamano 16384 - 500 = 15884
    TEST_ASSERT_EQUAL(mem.tabla_segmentos[1], (500 << 16) | 15884, "setear_tabla_segmento: entrada 1 base 500 tam 15884");
    // Entradas 2..7 en 0xFFFFFFFF
    int i;
    for (i = 2; i < TAM_SEGMENTOS; i++) {
        TEST_ASSERT_EQUAL(mem.tabla_segmentos[i], 0xFFFFFFFF, "setear_tabla_segmento: entradas 2..7 en 0xFFFFFFFF");
    }
    free(mem.mem_principal);

    // Test 2: inicializar_vmx
    Vmx vmx;
    inicializar_vmx(&vmx, 16, 0, 0);
    TEST_ASSERT_EQUAL(vmx.modo_debug, 0, "inicializar_vmx: modo_debug configurado");
    TEST_ASSERT_EQUAL(vmx.modo_disassembler, 0, "inicializar_vmx: modo_disassembler configurado");
    TEST_ASSERT_NOT_EQUAL((long long)vmx.abortar, 0, "inicializar_vmx: puntero abortar enlazado");
    for (i = 0; i < 32; i++) {
        TEST_ASSERT_EQUAL(vmx.registros[i], 0, "inicializar_vmx: registros inicializados en 0");
    }
    free(vmx.memoria.mem_principal);

    // Test 3: cargar_programa con archivo inexistente
    inicializar_vmx(&vmx, 16, 0, 0);
    TEST_ASSERT_ABORTA(cargar_programa(&vmx, "archivo_que_no_existe_404.vmx", 0), "cargar_programa: archivo inexistente aborta");

    // Test 4: cargar_programa con identificador magico invalido
    FILE *f_bad = fopen("bad_header.vmx", "wb");
    fwrite("CORRUP", 1, 6, f_bad);
    fclose(f_bad);
    TEST_ASSERT_ABORTA(cargar_programa(&vmx, "bad_header.vmx", 0), "cargar_programa: identificador magico invalido aborta");
    remove("bad_header.vmx");

    // Test 5: cargar_programa con version no compatible
    FILE *f_bad_ver = fopen("bad_ver.vmx", "wb");
    fwrite("VMX26", 1, 5, f_bad_ver);
    uint8_t v9 = 9;
    fwrite(&v9, 1, 1, f_bad_ver);
    fclose(f_bad_ver);
    TEST_ASSERT_ABORTA(cargar_programa(&vmx, "bad_ver.vmx", 0), "cargar_programa: version invalida aborta");
    remove("bad_ver.vmx");

    // Test 6: cargar_programa V1 sintetico
    FILE *f_v1 = fopen("dummy_v1.vmx", "wb");
    fwrite("VMX26", 1, 5, f_v1);
    uint8_t v1 = 1;
    fwrite(&v1, 1, 1, f_v1);
    uint8_t tam_cs_bytes[2] = {0x00, 0x02}; // 2 bytes de codigo
    fwrite(tam_cs_bytes, 1, 2, f_v1);
    uint8_t codigo_dummy[2] = {0x0F, 0x0E}; // STOP, RET
    fwrite(codigo_dummy, 1, 2, f_v1);
    fclose(f_v1);

    cargar_programa(&vmx, "dummy_v1.vmx", 0);
    TEST_ASSERT_EQUAL(vmx.registros[CS], 0x00000000, "cargar_programa V1: CS apunta a 0x00000000");
    TEST_ASSERT_EQUAL(vmx.registros[DS], 0x00010000, "cargar_programa V1: DS apunta a 0x00010000");
    TEST_ASSERT_EQUAL(vmx.registros[IP], 0x00000000, "cargar_programa V1: IP inicializado en CS");
    TEST_ASSERT_EQUAL(vmx.memoria.tabla_segmentos[0], 2, "cargar_programa V1: tamano de codigo 2 bytes");
    TEST_ASSERT_EQUAL(vmx.memoria.mem_principal[0], 0x0F, "cargar_programa V1: primer byte de codigo cargado");
    TEST_ASSERT_EQUAL(vmx.memoria.mem_principal[1], 0x0E, "cargar_programa V1: segundo byte de codigo cargado");
    remove("dummy_v1.vmx");
    free(vmx.memoria.mem_principal);

    // Test 7: cargar_programa V2 sintetico completo (Header V2 de 18 bytes + Codigo + Constantes)
    inicializar_vmx(&vmx, 16, 0, 0);
    FILE *f_v2 = fopen("dummy_v2.vmx", "wb");
    fwrite("VMX26", 1, 5, f_v2);
    uint8_t v2 = 2;
    fwrite(&v2, 1, 1, f_v2);
    // Header V2: tam_cs=4, tam_ds=20, tam_es=10, tam_ss=30, tam_ks=6, entry_point=2
    uint8_t header_v2_campos[12] = {
        0x00, 0x04, // CS = 4
        0x00, 0x14, // DS = 20
        0x00, 0x0A, // ES = 10
        0x00, 0x1E, // SS = 30
        0x00, 0x06, // KS = 6
        0x00, 0x02  // Entry point = 2
    };
    fwrite(header_v2_campos, 1, 12, f_v2);

    // Codigo CS (4 bytes)
    uint8_t codigo_v2[4] = {0x0F, 0x0E, 0x01, 0x02};
    fwrite(codigo_v2, 1, 4, f_v2);

    // Cadenas KS (6 bytes: "HOLA\0\0")
    fwrite("HOLA\0", 1, 6, f_v2);
    fclose(f_v2);

    cargar_programa(&vmx, "dummy_v2.vmx", 0);
    TEST_ASSERT_NOT_EQUAL(vmx.registros[CS], -1, "cargar_programa V2: CS configurado");
    TEST_ASSERT_NOT_EQUAL(vmx.registros[DS], -1, "cargar_programa V2: DS configurado");
    TEST_ASSERT_NOT_EQUAL(vmx.registros[ES], -1, "cargar_programa V2: ES configurado");
    TEST_ASSERT_NOT_EQUAL(vmx.registros[SS], -1, "cargar_programa V2: SS configurado");
    TEST_ASSERT_NOT_EQUAL(vmx.registros[KS], -1, "cargar_programa V2: KS configurado");
    // IP debe tener CS en alto y offset entry point (2) en bajo
    int cs_idx = (vmx.registros[CS] >> 16) & 0xFFFF;
    TEST_ASSERT_EQUAL(vmx.registros[IP], (cs_idx << 16) | 2, "cargar_programa V2: IP configurado en CS | entry_point");
    remove("dummy_v2.vmx");
    free(vmx.memoria.mem_principal);

    // Test 8: inicializar_subrutina_principal CON parametros
    inicializar_vmx(&vmx, 16, 0, 0);
    vmx.memoria.tabla_segmentos[1] = (100 << 16) | 20;
    vmx.registros[SS] = 0x00010000;
    vmx.registros[SP] = 0x00010014; // offset 20

    InfoParamSegment info_sub;
    info_sub.existe = 1;
    info_sub.tamano = 37;
    info_sub.argc = 2;
    info_sub.dir_argv = 0x00000015;

    inicializar_subrutina_principal(&vmx, info_sub);
    TEST_ASSERT_EQUAL(vmx.registros[SP], 0x00010008, "inicializar_subrutina_principal con params: SP decrementado en 12");
    TEST_ASSERT_EQUAL(leer_memoria(&vmx.memoria, 100 + 8, 4), -1, "inicializar_subrutina_principal: tope contiene RET (-1)");
    TEST_ASSERT_EQUAL(leer_memoria(&vmx.memoria, 100 + 12, 4), 2, "inicializar_subrutina_principal: SP+4 contiene argc=2");
    TEST_ASSERT_EQUAL(leer_memoria(&vmx.memoria, 100 + 16, 4), 0x00000015, "inicializar_subrutina_principal: SP+8 contiene *argv");

    // Test 9: inicializar_subrutina_principal SIN parametros (especificacion: RET=-1, argc=0, argv=-1)
    vmx.registros[SP] = 0x00010014; // offset 20
    InfoParamSegment info_sin_params;
    info_sin_params.existe = 0;
    info_sin_params.tamano = 0;
    info_sin_params.argc = 0;
    info_sin_params.dir_argv = 0xFFFFFFFF;

    inicializar_subrutina_principal(&vmx, info_sin_params);
    TEST_ASSERT_EQUAL(vmx.registros[SP], 0x00010008, "inicializar_subrutina_principal sin params: SP decrementado en 12");
    TEST_ASSERT_EQUAL(leer_memoria(&vmx.memoria, 100 + 8, 4), -1, "inicializar_subrutina_principal sin params: tope contiene RET (-1)");
    TEST_ASSERT_EQUAL(leer_memoria(&vmx.memoria, 100 + 12, 4), 0, "inicializar_subrutina_principal sin params: SP+4 contiene argc=0");
    TEST_ASSERT_EQUAL(leer_memoria(&vmx.memoria, 100 + 16, 4), -1, "inicializar_subrutina_principal sin params: SP+8 contiene argv=-1");

    // Test 10: inicializar_subrutina_principal con Stack Overflow
    vmx.registros[SP] = 0x00010008; // SP - 12 < SS -> aborta
    TEST_ASSERT_ABORTA(inicializar_subrutina_principal(&vmx, info_sub), "inicializar_subrutina_principal: desborde de pila aborta");

    free(vmx.memoria.mem_principal);
}
