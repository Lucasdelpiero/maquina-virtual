#include "test_helpers.h"
#include "../../src/vmi.h"
#include "../../src/inicializador.h"

void test_vmi_suite(void) {
    printf("\n--- TESTS: vmi.c ---\n");

    // Test 1: vmi_configurar_archivo, vmi_tiene_archivo y vmi_obtener_archivo
    vmi_configurar_archivo(NULL);
    TEST_ASSERT_EQUAL(vmi_tiene_archivo(), 0, "vmi_configurar_archivo: NULL limpia la ruta configurada");
    TEST_ASSERT_STR_EQUAL(vmi_obtener_archivo(), "", "vmi_obtener_archivo: devuelve cadena vacia cuando no hay ruta");

    vmi_configurar_archivo("test_imagen.vmi");
    TEST_ASSERT_EQUAL(vmi_tiene_archivo(), 1, "vmi_tiene_archivo: devuelve 1 cuando hay ruta");
    TEST_ASSERT_STR_EQUAL(vmi_obtener_archivo(), "test_imagen.vmi", "vmi_obtener_archivo: devuelve ruta configurada");

    // Test 2: Validacion de parametros al guardar y cargar
    TEST_ASSERT_EQUAL(guardar_imagen_vmi(NULL, "ruta.vmi", 16), 0, "guardar_imagen_vmi: vmx NULL devuelve 0");
    TEST_ASSERT_EQUAL(cargar_imagen_vmi(NULL, "ruta.vmi", NULL), 0, "cargar_imagen_vmi: vmx NULL devuelve 0");
    TEST_ASSERT_EQUAL(cargar_imagen_vmi((Vmx*)1, NULL, NULL), 0, "cargar_imagen_vmi: ruta NULL devuelve 0");
    TEST_ASSERT_EQUAL(cargar_imagen_vmi((Vmx*)1, "archivo_inexistente_123456.vmi", NULL), 0, "cargar_imagen_vmi: archivo inexistente devuelve 0");

    // Test 3: Round-trip guardar y cargar imagen con verificacion exhaustiva de los 32 registros
    Vmx vmx_guardar;
    inicializar_vmx(&vmx_guardar, 1, 0, 0); // 1 KiB para test rapido

    // Configuramos valores especificos en TODOS los 32 registros
    int i;
    for (i = 0; i < 32; i++) {
        vmx_guardar.registros[i] = (i * 100) + 7;
    }
    // Asignamos descriptores en las 8 entradas
    for (i = 0; i < TAM_SEGMENTOS; i++) {
        vmx_guardar.memoria.tabla_segmentos[i] = (i << 20) | (i * 10 + 5);
    }
    vmx_guardar.memoria.mem_principal[0] = 0xDE;
    vmx_guardar.memoria.mem_principal[1] = 0xAD;
    vmx_guardar.memoria.mem_principal[1023] = 0xEF;

    char ruta_temp[] = "test_temp_unitario.vmi";
    int guardado_ok = guardar_imagen_vmi(&vmx_guardar, ruta_temp, 1);
    TEST_ASSERT_EQUAL(guardado_ok, 1, "guardar_imagen_vmi: imagen guardada exitosamente");

    // Cargamos en una nueva estructura VMX
    Vmx vmx_cargar;
    inicializar_vmx(&vmx_cargar, 1, 0, 0);
    int mem_kib_leido = 0;
    int cargado_ok = cargar_imagen_vmi(&vmx_cargar, ruta_temp, &mem_kib_leido);
    TEST_ASSERT_EQUAL(cargado_ok, 1, "cargar_imagen_vmi: imagen cargada exitosamente");
    TEST_ASSERT_EQUAL(mem_kib_leido, 1, "cargar_imagen_vmi: tamano de memoria leido es 1 KiB");

    // Verificamos preservacion intacta de los 32 registros
    int todos_regs_ok = 1;
    for (i = 0; i < 32; i++) {
        if (vmx_cargar.registros[i] != (int32_t)((i * 100) + 7)) {
            todos_regs_ok = 0;
            break;
        }
    }
    TEST_ASSERT_EQUAL(todos_regs_ok, 1, "cargar_imagen_vmi: todos los 32 registros preservados con exactitud");

    // Verificamos las 8 entradas de la tabla de descriptores
    int todos_descs_ok = 1;
    for (i = 0; i < TAM_SEGMENTOS; i++) {
        if (vmx_cargar.memoria.tabla_segmentos[i] != (uint32_t)((i << 20) | (i * 10 + 5))) {
            todos_descs_ok = 0;
            break;
        }
    }
    TEST_ASSERT_EQUAL(todos_descs_ok, 1, "cargar_imagen_vmi: las 8 entradas de tabla_segmentos preservadas");

    // Verificamos la memoria
    TEST_ASSERT_EQUAL(vmx_cargar.memoria.mem_principal[0], 0xDE, "cargar_imagen_vmi: mem[0] restaurado");
    TEST_ASSERT_EQUAL(vmx_cargar.memoria.mem_principal[1], 0xAD, "cargar_imagen_vmi: mem[1] restaurado");
    TEST_ASSERT_EQUAL(vmx_cargar.memoria.mem_principal[1023], 0xEF, "cargar_imagen_vmi: mem[1023] restaurado");

    // Test 4: Archivo corrupto (magic invalido)
    FILE *f_bad = fopen("bad_magic.vmi", "wb");
    fwrite("XXXXX", 1, 5, f_bad);
    uint8_t v = 1;
    fwrite(&v, 1, 1, f_bad);
    fclose(f_bad);
    TEST_ASSERT_EQUAL(cargar_imagen_vmi(&vmx_cargar, "bad_magic.vmi", NULL), 0, "cargar_imagen_vmi: magic invalido rechazado");

    // Test 5: Archivo truncado (menos de 8 bytes de header)
    FILE *f_trunc = fopen("trunc.vmi", "wb");
    fwrite("VMI", 1, 3, f_trunc);
    fclose(f_trunc);
    TEST_ASSERT_EQUAL(cargar_imagen_vmi(&vmx_cargar, "trunc.vmi", NULL), 0, "cargar_imagen_vmi: archivo truncado < 8 bytes rechazado");

    // Test 6: Archivo con version != 1
    FILE *f_bad_ver = fopen("bad_ver.vmi", "wb");
    fwrite("VMI26", 1, 5, f_bad_ver);
    uint8_t v9 = 9;
    fwrite(&v9, 1, 1, f_bad_ver);
    uint8_t mem_kb[2] = {0, 1};
    fwrite(mem_kb, 1, 2, f_bad_ver);
    fclose(f_bad_ver);
    TEST_ASSERT_EQUAL(cargar_imagen_vmi(&vmx_cargar, "bad_ver.vmi", NULL), 0, "cargar_imagen_vmi: version != 1 rechazada");

    // Test 7: Redimension dinamica de memoria (guardar con 2 KiB y cargar en VMX inicializada en 1 KiB)
    Vmx vmx_grande;
    inicializar_vmx(&vmx_grande, 2, 0, 0); // 2 KiB
    vmx_grande.memoria.mem_principal[2047] = 0x77; // Byte en el limite superior de 2 KiB
    char ruta_resize[] = "test_resize.vmi";
    guardar_imagen_vmi(&vmx_grande, ruta_resize, 2);

    Vmx vmx_pequena;
    inicializar_vmx(&vmx_pequena, 1, 0, 0); // Inicializada en 1 KiB
    int mem_resize_leida = 0;
    int cargado_resize_ok = cargar_imagen_vmi(&vmx_pequena, ruta_resize, &mem_resize_leida);
    TEST_ASSERT_EQUAL(cargado_resize_ok, 1, "cargar_imagen_vmi: carga exitosa de imagen con memoria mayor");
    TEST_ASSERT_EQUAL(mem_resize_leida, 2, "cargar_imagen_vmi: tamano leido actualizado a 2 KiB");
    TEST_ASSERT_EQUAL(vmx_pequena.memoria.tam_mem_kib, 2, "cargar_imagen_vmi: tam_mem_kib interno actualizado a 2");
    TEST_ASSERT_EQUAL(vmx_pequena.memoria.mem_principal[2047], 0x77, "cargar_imagen_vmi: byte en 2047 restaurado tras realloc");

    // Limpieza de archivos temporales
    remove(ruta_temp);
    remove("bad_magic.vmi");
    remove("trunc.vmi");
    remove("bad_ver.vmi");
    remove(ruta_resize);

    free(vmx_guardar.memoria.mem_principal);
    free(vmx_cargar.memoria.mem_principal);
    free(vmx_grande.memoria.mem_principal);
    free(vmx_pequena.memoria.mem_principal);
}
