#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <setjmp.h>
#include "tests_helpers.h"
#include "../../src/vmx.h"
#include "../../src/inicializador.h"
#include "../../src/param_segment.h"
#include "../../src/vmi.h"
#include "../../src/depurador.h"
#include "../../src/errores.h"

// Variables para captura controlada de abortar en pruebas de excepciones
static jmp_buf s_jmp_abort_p2;
static int s_aborto_llamado = 0;
static char s_ultimo_mensaje_abort[256] = "";

static void mock_abortar_p2(char mensaje[]) {
    s_aborto_llamado = 1;
    if (mensaje != NULL) {
        strncpy(s_ultimo_mensaje_abort, mensaje, sizeof(s_ultimo_mensaje_abort) - 1);
        s_ultimo_mensaje_abort[sizeof(s_ultimo_mensaje_abort) - 1] = '\0';
    }
    longjmp(s_jmp_abort_p2, 1);
}

static void resolver_ruta_p2(const char relativa[], char ruta_final[]) {
    FILE *f;
    strcpy(ruta_final, relativa);
    f = fopen(ruta_final, "rb");
    if (f != NULL) {
        fclose(f);
        return;
    }
    sprintf(ruta_final, "tests/%s", relativa);
    f = fopen(ruta_final, "rb");
    if (f != NULL) {
        fclose(f);
        return;
    }
    sprintf(ruta_final, "../%s", relativa);
    f = fopen(ruta_final, "rb");
    if (f != NULL) {
        fclose(f);
        return;
    }
    strcpy(ruta_final, relativa);
}

static void cargar_y_preparar_p2(Vmx *vmx, const char ruta[], InfoParamSegment info_ps) {
    cargar_programa(vmx, (char*)ruta, info_ps.tamano);
    if (vmx->registros[SS] != -1) {
        inicializar_subrutina_principal(vmx, info_ps);
    }
}

// Crea un archivo binario V2 temporal valido para pruebas
static void crear_binario_v2_temporal(const char ruta[], int tam_datos, int tam_extra, int tam_pila) {
    FILE *f = fopen(ruta, "wb");
    if (f == NULL) return;

    // Header V2 (18 bytes)
    fwrite("VMX26", sizeof(char), 5, f); // 0-4
    uint8_t version = 2;
    fwrite(&version, sizeof(uint8_t), 1, f); // 5

    // tam_codigo = 6
    uint8_t tam_cod_be[2] = {0x00, 0x06};
    fwrite(tam_cod_be, sizeof(uint8_t), 2, f); // 6-7

    // tam_datos
    uint8_t tam_dat_be[2];
    tam_dat_be[0] = (uint8_t)((tam_datos >> 8) & 0xFF);
    tam_dat_be[1] = (uint8_t)(tam_datos & 0xFF);
    fwrite(tam_dat_be, sizeof(uint8_t), 2, f); // 8-9

    // tam_extra
    uint8_t tam_ext_be[2];
    tam_ext_be[0] = (uint8_t)((tam_extra >> 8) & 0xFF);
    tam_ext_be[1] = (uint8_t)(tam_extra & 0xFF);
    fwrite(tam_ext_be, sizeof(uint8_t), 2, f); // 10-11

    // tam_pila
    uint8_t tam_pil_be[2];
    tam_pil_be[0] = (uint8_t)((tam_pila >> 8) & 0xFF);
    tam_pil_be[1] = (uint8_t)(tam_pila & 0xFF);
    fwrite(tam_pil_be, sizeof(uint8_t), 2, f); // 12-13

    // tam_const = 13 (ej: "Hola mundo!\0\0")
    uint8_t tam_cst_be[2] = {0x00, 0x0D};
    fwrite(tam_cst_be, sizeof(uint8_t), 2, f); // 14-15

    // offset_entry_point = 0
    uint8_t entry_be[2] = {0x00, 0x00};
    fwrite(entry_be, sizeof(uint8_t), 2, f); // 16-17

    // Code Segment (6 bytes)
    uint8_t codigo[6] = {0x10, 0x00, 0x00, 0x2A, 0x0F, 0x00};
    fwrite(codigo, sizeof(uint8_t), 6, f);

    // Const Segment (13 bytes)
    const char const_str[13] = "Hola mundo!\0";
    fwrite(const_str, sizeof(char), 13, f);

    fclose(f);
}

// Crea un archivo temporal corrupto o con datos arbitrarios
static void crear_archivo_bytes(const char ruta[], const uint8_t bytes[], int tam) {
    FILE *f = fopen(ruta, "wb");
    if (f == NULL) return;
    if (tam > 0 && bytes != NULL) {
        fwrite(bytes, sizeof(uint8_t), tam, f);
    }
    fclose(f);
}

void test_binarios_parte2(void) {
    Vmx vmx;
    Vmx vmx2;
    char params[3][MAX_PARAM_LEN];
    InfoParamSegment info;
    char ruta_temp_v2[260];
    char ruta_temp_vmi[260];
    char ruta_temp_err[260];
    int tam_kib_leido;
    FILE *f_test;

    printf("\n--- TESTS: BINARIOS Y MODULOS PARTE II ---\n");

    // =========================================================================
    // 1. Tests de Param Segment (src/param_segment.c)
    // =========================================================================
    inicializar_vmx(&vmx, 16, 0, 0);

    // Caso A: Sin parametros (-p no provisto o nro_params == 0)
    info = construir_param_segment(&vmx, "prog.vmx", params, 0);
    ASSERT_EQUAL(info.existe, 0, "Param Segment sin parametros: existe == 0");
    ASSERT_EQUAL(info.tamano, 0, "Param Segment sin parametros: tamano == 0");
    ASSERT_EQUAL(info.argc, 0, "Param Segment sin parametros: argc == 0");
    ASSERT_EQUAL(info.dir_argv, 0xFFFFFFFF, "Param Segment sin parametros: dir_argv == 0xFFFFFFFF");
    ASSERT_EQUAL(vmx.registros[PS], -1, "Param Segment sin parametros: registro PS queda en -1 (0xFFFFFFFF)");

    // Caso B: Con parametros - Ejemplo exacto de la especificacion de Parte 2:
    // vmx prog.vmx -p prueba 01 2
    // Strings: prog.vmx\0 (9) + prueba\0 (7) + 01\0 (3) + 2\0 (2) = 21 bytes
    // Arreglo argv: 4 punteros de 4 bytes = 16 bytes. Total = 37 bytes.
    strcpy(params[0], "prueba");
    strcpy(params[1], "01");
    strcpy(params[2], "2");

    inicializar_vmx(&vmx, 16, 0, 0);
    info = construir_param_segment(&vmx, "prog.vmx", params, 3);

    ASSERT_EQUAL(info.existe, 1, "Param Segment con parametros: existe == 1");
    ASSERT_EQUAL(info.tamano, 37, "Param Segment ejemplo especificacion: tamano total == 37 bytes");
    ASSERT_EQUAL(info.argc, 4, "Param Segment ejemplo especificacion: argc == 4 (prog.vmx + 3 params)");
    ASSERT_EQUAL(info.dir_argv, 21, "Param Segment ejemplo especificacion: dir_argv == 21 (offset del arreglo argv)");
    ASSERT_EQUAL(vmx.registros[PS], 0x00000000, "Param Segment: registro PS inicializado a 0x00000000");
    ASSERT_EQUAL((int)(vmx.memoria.tabla_segmentos[0] & 0xFFFF), 37, "Param Segment: entrada 0 de tabla tiene tamano == 37");
    ASSERT_EQUAL((int)((vmx.memoria.tabla_segmentos[0] >> 16) & 0xFFFF), 0, "Param Segment: entrada 0 de tabla tiene base == 0");

    // Validacion de contenido en memoria fisica (strings contiguos terminados en \0)
    ASSERT(strcmp((char*)&vmx.memoria.mem_principal[0], "prog.vmx") == 0, "Param Segment: string 0 es 'prog.vmx\\0'");
    ASSERT(strcmp((char*)&vmx.memoria.mem_principal[9], "prueba") == 0, "Param Segment: string 1 es 'prueba\\0' en offset 9");
    ASSERT(strcmp((char*)&vmx.memoria.mem_principal[16], "01") == 0, "Param Segment: string 2 es '01\\0' en offset 16");
    ASSERT(strcmp((char*)&vmx.memoria.mem_principal[19], "2") == 0, "Param Segment: string 3 es '2\\0' en offset 19");

    // Validacion de punteros big-endian en arreglo argv
    // Puntero 0 -> offset 0: 0x00 0x00 0x00 0x00
    ASSERT_EQUAL(vmx.memoria.mem_principal[21], 0x00, "argv[0] byte 0 == 0x00");
    ASSERT_EQUAL(vmx.memoria.mem_principal[24], 0x00, "argv[0] byte 3 == 0x00");
    // Puntero 1 -> offset 9: 0x00 0x00 0x00 0x09
    ASSERT_EQUAL(vmx.memoria.mem_principal[25], 0x00, "argv[1] byte 0 == 0x00");
    ASSERT_EQUAL(vmx.memoria.mem_principal[28], 0x09, "argv[1] byte 3 == 0x09");
    // Puntero 2 -> offset 16: 0x00 0x00 0x00 0x10
    ASSERT_EQUAL(vmx.memoria.mem_principal[29], 0x00, "argv[2] byte 0 == 0x00");
    ASSERT_EQUAL(vmx.memoria.mem_principal[32], 0x10, "argv[2] byte 3 == 0x10");
    // Puntero 3 -> offset 19: 0x00 0x00 0x00 0x13
    ASSERT_EQUAL(vmx.memoria.mem_principal[33], 0x00, "argv[3] byte 0 == 0x00");
    ASSERT_EQUAL(vmx.memoria.mem_principal[36], 0x13, "argv[3] byte 3 == 0x13");

    // Caso C: Unico parametro - "app.vmx -p arg_unico"
    // Strings: app.vmx\0 (8 bytes) + arg_unico\0 (10 bytes) = 18 bytes
    // argv: 2 punteros de 4 bytes = 8 bytes. Total = 26 bytes.
    strcpy(params[0], "arg_unico");
    inicializar_vmx(&vmx, 16, 0, 0);
    info = construir_param_segment(&vmx, "app.vmx", params, 1);
    ASSERT_EQUAL(info.existe, 1, "Param Segment con 1 parametro: existe == 1");
    ASSERT_EQUAL(info.tamano, 26, "Param Segment con 1 parametro: tamano total == 26 bytes");
    ASSERT_EQUAL(info.argc, 2, "Param Segment con 1 parametro: argc == 2");
    ASSERT_EQUAL(info.dir_argv, 18, "Param Segment con 1 parametro: dir_argv == 18");
    ASSERT(strcmp((char*)&vmx.memoria.mem_principal[0], "app.vmx") == 0, "Param Segment: string 'app.vmx'");
    ASSERT(strcmp((char*)&vmx.memoria.mem_principal[8], "arg_unico") == 0, "Param Segment: string 'arg_unico' en offset 8");
    ASSERT_EQUAL(vmx.memoria.mem_principal[18 + 7], 0x08, "argv[1] apunta a offset 8 de arg_unico");

    // =========================================================================
    // 2. Tests de Imagenes VMI (src/vmi.c)
    // =========================================================================
    strcpy(ruta_temp_vmi, "tests/test_temp.vmi");
    inicializar_vmx(&vmx, 16, 0, 0);

    // Modificamos estado representativo
    vmx.registros[EAX] = 0x12345678;
    vmx.registros[EBX] = -12345;
    vmx.registros[IP]  = 0x00020040;
    vmx.registros[SP]  = 0x00051200;
    vmx.registros[CS]  = 0x00020000;
    vmx.registros[DS]  = 0x00030000;
    vmx.registros[PS]  = 0x00000000;
    vmx.memoria.tabla_segmentos[0] = 37;
    vmx.memoria.tabla_segmentos[1] = ((uint32_t)37 << 16) | 100;
    vmx.memoria.tabla_segmentos[2] = ((uint32_t)137 << 16) | 500;
    vmx.memoria.mem_principal[0] = 'V';
    vmx.memoria.mem_principal[1] = 'M';
    vmx.memoria.mem_principal[2] = 'I';
    vmx.memoria.mem_principal[137] = 0xAA;
    vmx.memoria.mem_principal[138] = 0xBB;

    ASSERT_EQUAL(guardar_imagen_vmi(&vmx, ruta_temp_vmi, 16), 1, "guardar_imagen_vmi: Guarda imagen correctamente");

    // Validar tamano exacto de archivo: 8 (header) + 128 (regs) + 32 (tabla) + 16384 (mem) = 16552 bytes
    f_test = fopen(ruta_temp_vmi, "rb");
    ASSERT(f_test != NULL, "Archivo VMI generado existe en disco");
    if (f_test != NULL) {
        fseek(f_test, 0, SEEK_END);
        long tam_arch_vmi = ftell(f_test);
        fclose(f_test);
        ASSERT_EQUAL((int)tam_arch_vmi, 16552, "Archivo VMI tiene tamano exacto segun especificacion (16552 bytes)");
    }

    // Restaurar sobre otra instancia limpia
    inicializar_vmx(&vmx2, 16, 0, 0);
    tam_kib_leido = 0;
    ASSERT_EQUAL(cargar_imagen_vmi(&vmx2, ruta_temp_vmi, &tam_kib_leido), 1, "cargar_imagen_vmi: Carga imagen correctamente");
    ASSERT_EQUAL(tam_kib_leido, 16, "VMI leido: Tamano de memoria en cabecera es 16 KiB");
    ASSERT_EQUAL(vmx2.registros[EAX], 0x12345678, "VMI restaurado: Registro EAX coincide exactamente");
    ASSERT_EQUAL(vmx2.registros[EBX], -12345, "VMI restaurado: Registro EBX coincide exactamente");
    ASSERT_EQUAL(vmx2.registros[IP], 0x00020040, "VMI restaurado: Registro IP coincide exactamente");
    ASSERT_EQUAL(vmx2.registros[SP], 0x00051200, "VMI restaurado: Registro SP coincide exactamente");
    ASSERT_EQUAL(vmx2.registros[CS], 0x00020000, "VMI restaurado: Registro CS coincide exactamente");
    ASSERT_EQUAL(vmx2.registros[PS], 0x00000000, "VMI restaurado: Registro PS coincide exactamente");
    ASSERT_EQUAL((int)vmx2.memoria.tabla_segmentos[0], 37, "VMI restaurado: Entrada 0 de tabla coincide");
    ASSERT_EQUAL((int)vmx2.memoria.tabla_segmentos[1], (int)(((uint32_t)37 << 16) | 100), "VMI restaurado: Entrada 1 de tabla coincide");
    ASSERT_EQUAL(vmx2.memoria.mem_principal[0], 'V', "VMI restaurado: Memoria fisica byte 0 coincide");
    ASSERT_EQUAL(vmx2.memoria.mem_principal[137], 0xAA, "VMI restaurado: Memoria fisica byte 137 coincide");

    remove(ruta_temp_vmi);

    // Caso B: VMI con 8 KiB de memoria (8 + 128 + 32 + 8192 = 8360 bytes)
    ASSERT_EQUAL(guardar_imagen_vmi(&vmx, ruta_temp_vmi, 8), 1, "guardar_imagen_vmi: Guarda imagen con 8 KiB");
    f_test = fopen(ruta_temp_vmi, "rb");
    if (f_test != NULL) {
        fseek(f_test, 0, SEEK_END);
        long tam_arch_8 = ftell(f_test);
        fclose(f_test);
        ASSERT_EQUAL((int)tam_arch_8, 8360, "Archivo VMI de 8 KiB tiene tamano exacto (8360 bytes)");
    }
    inicializar_vmx(&vmx2, 16, 0, 0);
    tam_kib_leido = 0;
    ASSERT_EQUAL(cargar_imagen_vmi(&vmx2, ruta_temp_vmi, &tam_kib_leido), 1, "cargar_imagen_vmi: Carga imagen de 8 KiB correctamente");
    ASSERT_EQUAL(tam_kib_leido, 8, "VMI leido: Tamano de memoria en cabecera es 8 KiB");
    remove(ruta_temp_vmi);

    // Caso C: Manejo de errores y rechazo de VMI invalido/danado
    ASSERT_EQUAL(cargar_imagen_vmi(&vmx, "tests/no_existe.vmi", &tam_kib_leido), 0,
                 "cargar_imagen_vmi: Archivo inexistente retorna 0 (falla limpiamente)");

    strcpy(ruta_temp_err, "tests/vmi_corrupto.vmi");
    // Identificador corrupto ("BAD26\1...")
    uint8_t bytes_bad_magic[10] = {'B', 'A', 'D', '2', '6', 1, 0, 16, 0, 0};
    crear_archivo_bytes(ruta_temp_err, bytes_bad_magic, 10);
    ASSERT_EQUAL(cargar_imagen_vmi(&vmx, ruta_temp_err, &tam_kib_leido), 0,
                 "cargar_imagen_vmi: Identificador de cabecera invalido retorna 0");
    remove(ruta_temp_err);

    // Version incompatible ("VMI26\2...")
    uint8_t bytes_bad_ver[10] = {'V', 'M', 'I', '2', '6', 2, 0, 16, 0, 0};
    crear_archivo_bytes(ruta_temp_err, bytes_bad_ver, 10);
    ASSERT_EQUAL(cargar_imagen_vmi(&vmx, ruta_temp_err, &tam_kib_leido), 0,
                 "cargar_imagen_vmi: Version incompatible retorna 0");
    remove(ruta_temp_err);

    // Archivo truncado (solo 30 bytes)
    uint8_t bytes_trunc[30] = {'V', 'M', 'I', '2', '6', 1, 0, 16, 0};
    crear_archivo_bytes(ruta_temp_err, bytes_trunc, 30);
    ASSERT_EQUAL(cargar_imagen_vmi(&vmx, ruta_temp_err, &tam_kib_leido), 0,
                 "cargar_imagen_vmi: Archivo truncado retorna 0");
    remove(ruta_temp_err);

    // Caso D: Guardar con ruta nula y sin archivo configurado
    vmi_configurar_archivo(NULL);
    ASSERT_EQUAL(guardar_imagen_vmi(&vmx, NULL, 0), 0,
                 "guardar_imagen_vmi: Sin ruta ni archivo configurado retorna 0");
    ASSERT_EQUAL(guardar_imagen_vmi(NULL, "dummy.vmi", 0), 0,
                 "guardar_imagen_vmi: Puntero VMX nulo retorna 0");

    // =========================================================================
    // 3. Tests de Carga V2 y Distribucion Contigua en Memoria (src/inicializador.c)
    // =========================================================================
    strcpy(ruta_temp_v2, "tests/test_temp_v2.vmx");
    crear_binario_v2_temporal(ruta_temp_v2, 100, 50, 200);

    // Caso A: Carga V2 sin Param Segment (-p ausente)
    // La distribucion fisica debe ser: Const Segment en 0, Code Segment en 13
    inicializar_vmx(&vmx, 16, 0, 0);
    cargar_programa(&vmx, ruta_temp_v2, 0);

    // Const Segment (13 bytes) cargado a partir de la direccion fisica 0
    ASSERT(strncmp((char*)&vmx.memoria.mem_principal[0], "Hola mundo!", 11) == 0,
           "Carga V2 sin -p: Const Segment ubicado en direccion fisica 0");
    // Code Segment (6 bytes) cargado a partir de la direccion fisica 13 (despues de const)
    ASSERT_EQUAL(vmx.memoria.mem_principal[13], 0x10, "Carga V2 sin -p: Code Segment ubicado en direccion fisica 13 (byte 0 es 0x10)");
    ASSERT_EQUAL(vmx.memoria.mem_principal[16], 0x2A, "Carga V2 sin -p: Code Segment byte 3 es 0x2A (42)");

    // Caso B: Carga V2 con Param Segment (-p presente)
    // Distribucion fisica: Param Segment en 0..36, Const Segment en 37..49, Code Segment en 50..55
    strcpy(params[0], "prueba");
    strcpy(params[1], "01");
    strcpy(params[2], "2");
    inicializar_vmx(&vmx, 16, 0, 0);
    info = construir_param_segment(&vmx, "prog.vmx", params, 3);
    ASSERT_EQUAL(info.tamano, 37, "Param Segment construido con tamano 37");

    // Ahora cargamos el programa V2
    cargar_programa(&vmx, ruta_temp_v2, info.tamano);

    // 1. Validar que Param Segment NO fue destruido ni sobreescrito en 0..36
    ASSERT(strcmp((char*)&vmx.memoria.mem_principal[0], "prog.vmx") == 0,
           "Proteccion de memoria fisica: Param Segment en 0x0 NO fue pisado por Code Segment");
    ASSERT(strcmp((char*)&vmx.memoria.mem_principal[9], "prueba") == 0,
           "Proteccion de memoria fisica: String 'prueba' permanece intacto");

    // 2. Validar que Const Segment fue ubicado contiguamente a continuacion de PS (posicion 37)
    ASSERT(strncmp((char*)&vmx.memoria.mem_principal[37], "Hola mundo!", 11) == 0,
           "Carga V2 con -p: Const Segment ubicado contiguamente tras Param Segment (posicion 37)");

    // 3. Validar que Code Segment fue ubicado contiguamente a continuacion de KS (posicion 37 + 13 = 50)
    ASSERT_EQUAL(vmx.memoria.mem_principal[50], 0x10,
                 "Carga V2 con -p: Code Segment ubicado contiguamente tras Const Segment (posicion 50)");
    ASSERT_EQUAL(vmx.memoria.mem_principal[53], 0x2A,
                 "Carga V2 con -p: Code Segment operando 42 presente en memoria fisica");

    remove(ruta_temp_v2);

    // Caso C: Deteccion de excepciones al cargar binarios defectuosos
    set_abortar_handler(mock_abortar_p2);

    // C.1 Archivo inexistente
    s_aborto_llamado = 0;
    if (setjmp(s_jmp_abort_p2) == 0) {
        cargar_programa(&vmx, "tests/no_existe_archivo.vmx", 0);
    }
    ASSERT_EQUAL(s_aborto_llamado, 1, "cargar_programa: Archivo inexistente dispara abortar");

    // C.2 Identificador invalido ("BAD26...")
    strcpy(ruta_temp_err, "tests/vmx_bad_ident.vmx");
    uint8_t bad_ident[6] = {'B', 'A', 'D', '2', '6', 2};
    crear_archivo_bytes(ruta_temp_err, bad_ident, 6);
    s_aborto_llamado = 0;
    if (setjmp(s_jmp_abort_p2) == 0) {
        cargar_programa(&vmx, ruta_temp_err, 0);
    }
    ASSERT_EQUAL(s_aborto_llamado, 1, "cargar_programa: Identificador invalido dispara abortar");
    remove(ruta_temp_err);

    // C.3 Version incompatible (version 9)
    strcpy(ruta_temp_err, "tests/vmx_bad_ver.vmx");
    uint8_t bad_ver[6] = {'V', 'M', 'X', '2', '6', 9};
    crear_archivo_bytes(ruta_temp_err, bad_ver, 6);
    s_aborto_llamado = 0;
    if (setjmp(s_jmp_abort_p2) == 0) {
        cargar_programa(&vmx, ruta_temp_err, 0);
    }
    ASSERT_EQUAL(s_aborto_llamado, 1, "cargar_programa: Version incompatible dispara abortar");
    remove(ruta_temp_err);

    // C.4 Memoria insuficiente (pide 30000 bytes de datos cuando el maximo es 16384)
    strcpy(ruta_temp_err, "tests/vmx_mem_overflow.vmx");
    crear_binario_v2_temporal(ruta_temp_err, 30000, 0, 0);
    s_aborto_llamado = 0;
    if (setjmp(s_jmp_abort_p2) == 0) {
        cargar_programa(&vmx, ruta_temp_err, 0);
    }
    ASSERT_EQUAL(s_aborto_llamado, 1, "cargar_programa: Binario que excede memoria disponible dispara abortar");
    remove(ruta_temp_err);

    set_abortar_handler(NULL);

    // =========================================================================
    // 4. Tests de ip_valido Dinamico con Segmento CS (src/vmx.c)
    // =========================================================================
    inicializar_vmx(&vmx, 16, 0, 0);
    // Asignamos CS al segmento 2 con tamano 50 bytes (base 100)
    vmx.registros[CS] = 0x00020000;
    vmx.memoria.tabla_segmentos[2] = ((uint32_t)100 << 16) | 50;

    // IP apuntando al inicio del segmento de codigo (offset 0)
    vmx.registros[IP] = 0x00020000;
    ASSERT_EQUAL(ip_valido(&vmx), 1, "ip_valido: IP en offset 0 de segmento CS=2 es valido");

    // IP dentro del limite del segmento de codigo (offset 49)
    vmx.registros[IP] = 0x00020031;
    ASSERT_EQUAL(ip_valido(&vmx), 1, "ip_valido: IP en offset 49 (dentro del limite 50) es valido");

    // IP fuera del limite del segmento de codigo (offset 50)
    vmx.registros[IP] = 0x00020032;
    ASSERT_EQUAL(ip_valido(&vmx), 0, "ip_valido: IP en offset 50 (fuera del limite) no es valido");

    // IP apuntando a otro segmento (segmento 0 cuando CS es 2)
    vmx.registros[IP] = 0x00000000;
    ASSERT_EQUAL(ip_valido(&vmx), 0, "ip_valido: IP apuntando a segmento distinto a CS no es valido");

    // IP en -1 (STOP)
    vmx.registros[IP] = -1;
    ASSERT_EQUAL(ip_valido(&vmx), 0, "ip_valido: IP en -1 (STOP) detecta fin de ciclo");

    // Segmento CS invalido en tabla (0xFFFFFFFF)
    vmx.registros[CS] = 0x00050000;
    vmx.registros[IP] = 0x00050000;
    vmx.memoria.tabla_segmentos[5] = 0xFFFFFFFF;
    ASSERT_EQUAL(ip_valido(&vmx), 0, "ip_valido: CS apuntando a entrada de tabla invalida retorna 0");

    // =========================================================================
    // 5. Tests del Modulo Depurador (src/depurador.c)
    // =========================================================================
    // Sin archivo VMI configurado, no debe intentar abrir ni escribir archivo
    vmi_configurar_archivo(NULL);
    ASSERT_EQUAL(vmi_tiene_archivo(), 0, "vmi_tiene_archivo() retorna 0 cuando no hay archivo configurado");
    inicializar_vmx(&vmx, 16, 0, 0);
    depurador_manejar_breakpoint(&vmx); // Debe ignorarse limpiamente segun la especificacion

    // Con archivo VMI configurado
    vmi_configurar_archivo("test_debug.vmi");
    ASSERT_EQUAL(vmi_tiene_archivo(), 1, "vmi_tiene_archivo() retorna 1 cuando hay archivo configurado");
    ASSERT(strcmp(vmi_obtener_archivo(), "test_debug.vmi") == 0, "vmi_obtener_archivo() retorna la ruta asignada");
    vmi_configurar_archivo(NULL);

    // depurador_verificar_paso_a_paso sin modo paso a paso activo es no-op seguro
    depurador_verificar_paso_a_paso(&vmx);
    ASSERT(1, "depurador_verificar_paso_a_paso: Ejecuta limpiamente sin modo activo");

    // =========================================================================
    // 6. Ejecucion de Binarios Reales de Parte II (.vmx)
    // =========================================================================
    char ruta_bin[260];
    InfoParamSegment ps_vacia = {0, 0, 0, 0xFFFFFFFF};

    // Basicos Parte 2
    // 01_push_pop_reg.vmx: PUSH EAX (12345), POP EBX
    inicializar_vmx(&vmx, 16, 0, 0);
    resolver_ruta_p2("vmx/p2_basicos/01_push_pop_reg.vmx", ruta_bin);
    cargar_y_preparar_p2(&vmx, ruta_bin, ps_vacia);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EBX, 12345, "01_push_pop_reg.vmx EBX = 12345");

    // 02_push_pop_inmediato.vmx: PUSH 9876, POP ECX
    inicializar_vmx(&vmx, 16, 0, 0);
    resolver_ruta_p2("vmx/p2_basicos/02_push_pop_inmediato.vmx", ruta_bin);
    cargar_y_preparar_p2(&vmx, ruta_bin, ps_vacia);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, ECX, 9876, "02_push_pop_inmediato.vmx ECX = 9876");

    // 03_push_pop_memoria.vmx: PUSH [0] (555), POP [4], MOV EAX, [4]
    inicializar_vmx(&vmx, 16, 0, 0);
    resolver_ruta_p2("vmx/p2_basicos/03_push_pop_memoria.vmx", ruta_bin);
    cargar_y_preparar_p2(&vmx, ruta_bin, ps_vacia);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 555, "03_push_pop_memoria.vmx EAX = 555");

    // 04_sectores_registro_al_ah.vmx: AL = 0x12, AH = 0x34 -> EAX = 0x3412
    inicializar_vmx(&vmx, 16, 0, 0);
    resolver_ruta_p2("vmx/p2_basicos/04_sectores_registro_al_ah.vmx", ruta_bin);
    cargar_y_preparar_p2(&vmx, ruta_bin, ps_vacia);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 0x3412, "04_sectores_registro_al_ah.vmx EAX = 0x3412");

    // 05_sectores_registro_ax.vmx: AX = 0x5678 -> EAX = 0x5678
    inicializar_vmx(&vmx, 16, 0, 0);
    resolver_ruta_p2("vmx/p2_basicos/05_sectores_registro_ax.vmx", ruta_bin);
    cargar_y_preparar_p2(&vmx, ruta_bin, ps_vacia);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 0x5678, "05_sectores_registro_ax.vmx EAX = 0x5678");

    // 06_modificador_memoria_byte.vmx: b[0] = 0x41, AL = b[0] -> AL = 0x41
    inicializar_vmx(&vmx, 16, 0, 0);
    resolver_ruta_p2("vmx/p2_basicos/06_modificador_memoria_byte.vmx", ruta_bin);
    cargar_y_preparar_p2(&vmx, ruta_bin, ps_vacia);
    ejecutar_vmx(&vmx);
    ASSERT_EQUAL(vmx.registros[EAX] & 0xFF, 0x41, "06_modificador_memoria_byte.vmx AL = 0x41");

    // 07_modificador_memoria_word.vmx: w[0] = 0x1234, AX = w[0] -> AX = 0x1234
    inicializar_vmx(&vmx, 16, 0, 0);
    resolver_ruta_p2("vmx/p2_basicos/07_modificador_memoria_word.vmx", ruta_bin);
    cargar_y_preparar_p2(&vmx, ruta_bin, ps_vacia);
    ejecutar_vmx(&vmx);
    ASSERT_EQUAL(vmx.registros[EAX] & 0xFFFF, 0x1234, "07_modificador_memoria_word.vmx AX = 0x1234");

    // 08_modificador_memoria_long.vmx: l[0] = 0x11223344, EBX = l[0]
    inicializar_vmx(&vmx, 16, 0, 0);
    resolver_ruta_p2("vmx/p2_basicos/08_modificador_memoria_long.vmx", ruta_bin);
    cargar_y_preparar_p2(&vmx, ruta_bin, ps_vacia);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EBX, 0x11223344, "08_modificador_memoria_long.vmx EBX = 0x11223344");

    // 09_call_ret_minimo.vmx: CALL sub_simple -> EAX = 777, RET
    inicializar_vmx(&vmx, 16, 0, 0);
    resolver_ruta_p2("vmx/p2_basicos/09_call_ret_minimo.vmx", ruta_bin);
    cargar_y_preparar_p2(&vmx, ruta_bin, ps_vacia);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 777, "09_call_ret_minimo.vmx EAX = 777");

    // 11_constante_equ_inmediata.vmx: EAX = 100 + 0x20 = 132
    inicializar_vmx(&vmx, 16, 0, 0);
    resolver_ruta_p2("vmx/p2_basicos/11_constante_equ_inmediata.vmx", ruta_bin);
    cargar_y_preparar_p2(&vmx, ruta_bin, ps_vacia);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 132, "11_constante_equ_inmediata.vmx EAX = 132");

    // Completos Parte 2
    // 01_subrutinas_anidadas.vmx: main -> func_a (10) -> func_b (25) = 35 en EDX
    inicializar_vmx(&vmx, 16, 0, 0);
    resolver_ruta_p2("vmx/p2_completos/01_subrutinas_anidadas.vmx", ruta_bin);
    cargar_y_preparar_p2(&vmx, ruta_bin, ps_vacia);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EDX, 35, "01_subrutinas_anidadas.vmx EDX = 35");

    // 02_pasaje_parametros_pila.vmx: sumar(27, 15) = 42 en EBX
    inicializar_vmx(&vmx, 16, 0, 0);
    resolver_ruta_p2("vmx/p2_completos/02_pasaje_parametros_pila.vmx", ruta_bin);
    cargar_y_preparar_p2(&vmx, ruta_bin, ps_vacia);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EBX, 42, "02_pasaje_parametros_pila.vmx EBX = 42");

    // 04_manipulacion_strings_memoria.vmx: longitud de 'Arquitectura2026' = 16
    inicializar_vmx(&vmx, 16, 0, 0);
    resolver_ruta_p2("vmx/p2_completos/04_manipulacion_strings_memoria.vmx", ruta_bin);
    cargar_y_preparar_p2(&vmx, ruta_bin, ps_vacia);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 16, "04_manipulacion_strings_memoria.vmx longitud = 16");

    // 05_subrutina_factorial_recursivo.vmx: 5! = 120
    inicializar_vmx(&vmx, 16, 0, 0);
    resolver_ruta_p2("vmx/p2_completos/05_subrutina_factorial_recursivo.vmx", ruta_bin);
    cargar_y_preparar_p2(&vmx, ruta_bin, ps_vacia);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EBX, 120, "05_subrutina_factorial_recursivo.vmx 5! = 120");

    // 06_subrutina_dig_max_oficial.vmx: dig_max(3842) = 8
    inicializar_vmx(&vmx, 16, 0, 0);
    resolver_ruta_p2("vmx/p2_completos/06_subrutina_dig_max_oficial.vmx", ruta_bin);
    cargar_y_preparar_p2(&vmx, ruta_bin, ps_vacia);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EBX, 8, "06_subrutina_dig_max_oficial.vmx max_dig = 8");

    // 07_modificadores_combinados.vmx: EL pasa de 0x0F a 0x1E
    inicializar_vmx(&vmx, 16, 0, 0);
    resolver_ruta_p2("vmx/p2_completos/07_modificadores_combinados.vmx", ruta_bin);
    cargar_y_preparar_p2(&vmx, ruta_bin, ps_vacia);
    ejecutar_vmx(&vmx);
    ASSERT_EQUAL(vmx.registros[EEX] & 0xFF, 0x1E, "07_modificadores_combinados.vmx EL = 0x1E");

    // Avanzados Parte 2
    // 01_arbol_binario_extra.vmx: buscar 60 -> EBX = 1, buscar 99 -> ECX = 0
    inicializar_vmx(&vmx, 16, 0, 0);
    resolver_ruta_p2("vmx/p2_avanzados/01_arbol_binario_extra.vmx", ruta_bin);
    cargar_y_preparar_p2(&vmx, ruta_bin, ps_vacia);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EBX, 1, "01_arbol_binario_extra.vmx busqueda 60 exitosa (1)");
    assert_registro(&vmx, ECX, 0, "01_arbol_binario_extra.vmx busqueda 99 fallida (0)");

    // 02_arbol_n_ario_extra.vmx: suma de todos los nodos del arbol n-ario = 47
    inicializar_vmx(&vmx, 16, 0, 0);
    resolver_ruta_p2("vmx/p2_avanzados/02_arbol_n_ario_extra.vmx", ruta_bin);
    cargar_y_preparar_p2(&vmx, ruta_bin, ps_vacia);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EBX, 47, "02_arbol_n_ario_extra.vmx suma arbol n-ario = 47");

    // Guia de ejercicios: Ejercicio 11 (elim_valor: valor 4 devuelve 5)
    inicializar_vmx(&vmx, 16, 0, 0);
    resolver_ruta_p2("vmx/guia_de_ejercicios/ej11/main_ej11.vmx", ruta_bin);
    cargar_y_preparar_p2(&vmx, ruta_bin, ps_vacia);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 5, "Guia Ejercicio 11: elim_valor para valor 4 devuelve 5 eliminados");

    // Guia de ejercicios: Ejercicio 12 (cant_caminos en digrafo aciclico)
    inicializar_vmx(&vmx, 16, 0, 0);
    resolver_ruta_p2("vmx/guia_de_ejercicios/ej12/main_ej12.vmx", ruta_bin);
    cargar_y_preparar_p2(&vmx, ruta_bin, ps_vacia);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 3, "Guia Ejercicio 12: cant_caminos en DAG devuelve 3 caminos");

    // Casos conflictivos Parte 2
    set_abortar_handler(mock_abortar_p2);

    // 03_stack_overflow.vmx
    inicializar_vmx(&vmx, 16, 0, 0);
    resolver_ruta_p2("vmx/conflictivos/03_stack_overflow.vmx", ruta_bin);
    cargar_y_preparar_p2(&vmx, ruta_bin, ps_vacia);
    s_aborto_llamado = 0;
    s_ultimo_mensaje_abort[0] = '\0';
    if (setjmp(s_jmp_abort_p2) == 0) {
        ejecutar_vmx(&vmx);
    }
    ASSERT(s_aborto_llamado == 1, "03_stack_overflow.vmx dispara llamada a abortar");
    ASSERT(strstr(s_ultimo_mensaje_abort, "STACK OVERFLOW") != NULL, "Mensaje reporta 'STACK OVERFLOW'");

    // 04_stack_underflow.vmx
    inicializar_vmx(&vmx, 16, 0, 0);
    resolver_ruta_p2("vmx/conflictivos/04_stack_underflow.vmx", ruta_bin);
    cargar_y_preparar_p2(&vmx, ruta_bin, ps_vacia);
    s_aborto_llamado = 0;
    s_ultimo_mensaje_abort[0] = '\0';
    if (setjmp(s_jmp_abort_p2) == 0) {
        ejecutar_vmx(&vmx);
    }
    ASSERT(s_aborto_llamado == 1, "04_stack_underflow.vmx dispara llamada a abortar");
    ASSERT(strstr(s_ultimo_mensaje_abort, "STACK UNDERFLOW") != NULL, "Mensaje reporta 'STACK UNDERFLOW'");

    set_abortar_handler(NULL);
}
