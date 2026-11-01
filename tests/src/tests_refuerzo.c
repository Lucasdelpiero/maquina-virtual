#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <setjmp.h>
#include "tests_helpers.h"
#include "../../src/vmx.h"
#include "../../src/memoria.h"
#include "../../src/operandos.h"
#include "../../src/operadores.h"
#include "../../src/inicializador.h"
#include "../../src/param_segment.h"
#include "../../src/vmi.h"
#include "../../src/errores.h"

// Mecanismo de captura de excepciones controladas para los tests de limites
static jmp_buf s_jmp_abort_refuerzo;
static int s_aborto_disparado = 0;
static char s_msg_aborto[256] = "";

static void mock_abortar_refuerzo(char mensaje[]) {
    s_aborto_disparado = 1;
    if (mensaje != NULL) {
        strncpy(s_msg_aborto, mensaje, sizeof(s_msg_aborto) - 1);
        s_msg_aborto[sizeof(s_msg_aborto) - 1] = '\0';
    }
    longjmp(s_jmp_abort_refuerzo, 1);
}

static void resolver_ruta_refuerzo(char relativa[], char ruta_final[]) {
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

// Crea un archivo temporal con cabecera V2 para pruebas de memoria y limites
static void crear_archivo_v2(char ruta[], int tam_cod, int tam_dat, int tam_ext, int tam_stk, int tam_cst) {
    FILE *f = fopen(ruta, "wb");
    int i;
    uint8_t version = 2;
    uint8_t buffer[2];

    if (f == NULL) return;

    // Identificador 'VMX26'
    fwrite("VMX26", sizeof(char), 5, f);

    // Version 2
    fwrite(&version, sizeof(uint8_t), 1, f);

    // Tamano de codigo (big-endian)
    buffer[0] = (uint8_t)((tam_cod >> 8) & 0xFF);
    buffer[1] = (uint8_t)(tam_cod & 0xFF);
    fwrite(buffer, sizeof(uint8_t), 2, f);

    // Tamano de datos
    buffer[0] = (uint8_t)((tam_dat >> 8) & 0xFF);
    buffer[1] = (uint8_t)(tam_dat & 0xFF);
    fwrite(buffer, sizeof(uint8_t), 2, f);

    // Tamano extra
    buffer[0] = (uint8_t)((tam_ext >> 8) & 0xFF);
    buffer[1] = (uint8_t)(tam_ext & 0xFF);
    fwrite(buffer, sizeof(uint8_t), 2, f);

    // Tamano stack
    buffer[0] = (uint8_t)((tam_stk >> 8) & 0xFF);
    buffer[1] = (uint8_t)(tam_stk & 0xFF);
    fwrite(buffer, sizeof(uint8_t), 2, f);

    // Tamano const
    buffer[0] = (uint8_t)((tam_cst >> 8) & 0xFF);
    buffer[1] = (uint8_t)(tam_cst & 0xFF);
    fwrite(buffer, sizeof(uint8_t), 2, f);

    // Entry point (0)
    buffer[0] = 0;
    buffer[1] = 0;
    fwrite(buffer, sizeof(uint8_t), 2, f);

    // Escribir bytes de codigo (instruccion STOP o RET: 0x0E o 0x0F)
    for (i = 0; i < tam_cod; i++) {
        uint8_t b = (i == tam_cod - 1) ? 0x0E : 0x00;
        fwrite(&b, sizeof(uint8_t), 1, f);
    }

    // Escribir bytes de const segment si existen
    for (i = 0; i < tam_cst; i++) {
        uint8_t b = (uint8_t)('A' + (i % 26));
        fwrite(&b, sizeof(uint8_t), 1, f);
    }

    fclose(f);
}

// ----------------------------------------------------------------------------
// 1. Tests de casos limite de memoria y bordes
// ----------------------------------------------------------------------------
void correr_tests_refuerzo_memoria_bordes(void) {
    Vmx vmx;
    char ruta_temp[128];
    int limite_1k;
    int tam_total_ajustado;
    Memoria mem_test;
    int dir_fisica;

    printf("\n--- TESTS REFUERZO: Casos Limite de Memoria y Bordes ---\n");

    strcpy(ruta_temp, "temp_limite_mem.vmx");
    set_abortar_handler(mock_abortar_refuerzo);

    // 1.1 Proceso que ocupa EXACTAMENTE 1 KiB (1024 bytes) en memoria de 1 KiB
    // Code=100, Const=24, Data=400, Extra=300, Stack=200 -> Total = 1024 bytes
    limite_1k = 1024;
    tam_total_ajustado = 100 + 24 + 400 + 300 + 200;
    ASSERT_EQUAL(tam_total_ajustado, limite_1k, "Configuracion de proceso ocupa exactamente 1024 bytes");

    crear_archivo_v2(ruta_temp, 100, 400, 300, 200, 24);
    inicializar_vmx(&vmx, 1, 0, 0); // m=1 KiB

    s_aborto_disparado = 0;
    if (setjmp(s_jmp_abort_refuerzo) == 0) {
        cargar_programa(&vmx, ruta_temp, 0);
    }
    ASSERT(s_aborto_disparado == 0, "Proceso que entra exactamente en 1024 bytes carga sin error");
    ASSERT_EQUAL(vmx.memoria.tam_mem_kib, 1, "Tamano de memoria principal es 1 KiB");

    // 1.2 Proceso que excede por 1 SOLO byte la memoria disponible (1025 bytes en 1 KiB)
    // Code=100, Const=25, Data=400, Extra=300, Stack=200 -> Total = 1025 bytes
    crear_archivo_v2(ruta_temp, 100, 400, 300, 200, 25);
    inicializar_vmx(&vmx, 1, 0, 0); // m=1 KiB

    s_aborto_disparado = 0;
    s_msg_aborto[0] = '\0';
    if (setjmp(s_jmp_abort_refuerzo) == 0) {
        cargar_programa(&vmx, ruta_temp, 0);
    }
    ASSERT(s_aborto_disparado == 1, "Proceso de 1025 bytes en memoria de 1 KiB dispara abortar");
    ASSERT(strstr(s_msg_aborto, "Memoria insuficiente") != NULL, "Mensaje de error indica 'Memoria insuficiente'");

    // 1.3 Memoria maxima permitida (64 KiB = 65536 bytes)
    // Proceso grande: Code=5000, Const=1000, Data=20000, Extra=20000, Stack=15000 -> Total = 61000 bytes
    crear_archivo_v2(ruta_temp, 5000, 20000, 20000, 15000, 1000);
    inicializar_vmx(&vmx, 64, 0, 0); // m=64 KiB

    s_aborto_disparado = 0;
    if (setjmp(s_jmp_abort_refuerzo) == 0) {
        cargar_programa(&vmx, ruta_temp, 0);
    }
    ASSERT(s_aborto_disparado == 0, "Proceso de 61000 bytes en memoria de 64 KiB carga correctamente");
    ASSERT_EQUAL(vmx.memoria.tam_mem_kib, 64, "Tamano de memoria principal configurado en 64 KiB");

    // 1.4 Segmentos con tamano 0 (solo Code y Stack presentes, los demas ausentes)
    // PS=0, KS=0, CS=50, DS=0, ES=0, SS=100
    inicializar_vmx(&vmx, 16, 0, 0);
    crear_segmentos(&vmx.memoria, 0, 0, 50, 0, 0, 100);

    // Solo deben haberse creado 2 entradas en la tabla de descriptores
    ASSERT_EQUAL((int)(vmx.memoria.tabla_segmentos[0] & 0xFFFF), 50, "Entrada 0 en tabla es CS con tamano 50");
    ASSERT_EQUAL((int)((vmx.memoria.tabla_segmentos[0] >> 16) & 0xFFFF), 0, "Entrada 0 base fisica es 0");
    ASSERT_EQUAL((int)(vmx.memoria.tabla_segmentos[1] & 0xFFFF), 100, "Entrada 1 en tabla es SS con tamano 100");
    ASSERT_EQUAL((int)((vmx.memoria.tabla_segmentos[1] >> 16) & 0xFFFF), 50, "Entrada 1 base fisica es 50 (contigua tras CS)");
    ASSERT_EQUAL(vmx.memoria.tabla_segmentos[2], 0xFFFFFFFF, "Entrada 2 en tabla queda en 0xFFFFFFFF");
    ASSERT_EQUAL(vmx.memoria.tabla_segmentos[7], 0xFFFFFFFF, "Entrada 7 en tabla queda en 0xFFFFFFFF");

    // Registros de segmentos ausentes deben estar fijados estrictamente en -1
    ASSERT_EQUAL(vmx.registros[PS], -1, "Registro PS ausente queda en -1");
    ASSERT_EQUAL(vmx.registros[KS], -1, "Registro KS ausente queda en -1");
    ASSERT_EQUAL(vmx.registros[DS], -1, "Registro DS ausente queda en -1");
    ASSERT_EQUAL(vmx.registros[ES], -1, "Registro ES ausente queda en -1");
    ASSERT_EQUAL(vmx.registros[CS], 0x00000000, "Registro CS presente apunta a entrada 0 (0x00000000)");
    ASSERT_EQUAL(vmx.registros[SS], 0x00010000, "Registro SS presente apunta a entrada 1 (0x00010000)");
    ASSERT_EQUAL(vmx.registros[SP], 0x00010064, "Registro SP apunta al tope fuera de pila (SS + 100)");

    // 1.5 Accesos a memoria en limites exactos de segmento
    inicializar_memoria(&mem_test, 16);
    // Segmento 0: base=100, tamano=20 bytes (indices validos: 0..19)
    mem_test.tabla_segmentos[0] = (100 << 16) | 20;

    // Ultimo byte valido: offset 19 con cant_bytes=1
    dir_fisica = traducir_direccion(&mem_test, 0x00000013, 1);
    ASSERT_EQUAL(dir_fisica, 119, "Ultimo byte de segmento (offset 19) traduce a dir fisica 119");

    // Acceso desbordante en el limite: offset 19 con cant_bytes=2 (excede tamano 20)
    s_aborto_disparado = 0;
    if (setjmp(s_jmp_abort_refuerzo) == 0) {
        traducir_direccion(&mem_test, 0x00000013, 2);
    }
    ASSERT(s_aborto_disparado == 1, "Lectura de 2 bytes en offset 19 (tamano 20) dispara fallo de segmento");

    // Acceso desbordante con 4 bytes: offset 17 con cant_bytes=4 (17+4=21 > 20)
    s_aborto_disparado = 0;
    if (setjmp(s_jmp_abort_refuerzo) == 0) {
        traducir_direccion(&mem_test, 0x00000011, 4);
    }
    ASSERT(s_aborto_disparado == 1, "Lectura de 4 bytes en offset 17 (tamano 20) dispara fallo de segmento");

    // Acceso con offset negativo
    s_aborto_disparado = 0;
    if (setjmp(s_jmp_abort_refuerzo) == 0) {
        traducir_direccion(&mem_test, (int32_t)0x0000FFFF, 1);
    }
    ASSERT(s_aborto_disparado == 1, "Acceso con offset 0xFFFF fuera de limite dispara fallo de segmento");

    // Acceso a segmento no asignado (entrada 4 que tiene 0xFFFFFFFF)
    s_aborto_disparado = 0;
    if (setjmp(s_jmp_abort_refuerzo) == 0) {
        traducir_direccion(&mem_test, 0x00040000, 1);
    }
    ASSERT(s_aborto_disparado == 1, "Acceso a segmento no asignado dispara fallo de segmento");

    // 1.6 Casos borde exactos de Pila (Stack Segment de 16 bytes: caben exactamente 4 elementos)
    inicializar_vmx(&vmx, 16, 0, 0);
    crear_segmentos(&vmx.memoria, 0, 0, 32, 32, 0, 16); // SS en entrada 2, tamano 16

    // Estado inicial: pila vacia, SP == SS + 16
    ASSERT_EQUAL(vmx.registros[SP], vmx.registros[SS] + 16, "Pila recien inicializada tiene SP en SS + tamano");

    // Intento de POP en pila vacia -> STACK UNDERFLOW inmediato
    s_aborto_disparado = 0;
    s_msg_aborto[0] = '\0';
    if (setjmp(s_jmp_abort_refuerzo) == 0) {
        sacar_pila(&vmx);
    }
    ASSERT(s_aborto_disparado == 1, "Extraccion en pila vacia dispara STACK UNDERFLOW");
    ASSERT(strstr(s_msg_aborto, "STACK UNDERFLOW") != NULL, "Mensaje confirma STACK UNDERFLOW");

    // Insertar 4 elementos de 4 bytes (llenando exactamente los 16 bytes)
    vmx.registros[OP1] = (TIPO_INMEDIATO << 24) | 10;
    op_push(&vmx);
    vmx.registros[OP1] = (TIPO_INMEDIATO << 24) | 20;
    op_push(&vmx);
    vmx.registros[OP1] = (TIPO_INMEDIATO << 24) | 30;
    op_push(&vmx);
    vmx.registros[OP1] = (TIPO_INMEDIATO << 24) | 40;
    op_push(&vmx);

    ASSERT_EQUAL(vmx.registros[SP], vmx.registros[SS], "Tras 4 PUSH en pila de 16 bytes, SP alcanza exactamente SS (pila llena)");

    // El 5to PUSH debe disparar STACK OVERFLOW
    s_aborto_disparado = 0;
    s_msg_aborto[0] = '\0';
    if (setjmp(s_jmp_abort_refuerzo) == 0) {
        vmx.registros[OP1] = (TIPO_INMEDIATO << 24) | 50;
        op_push(&vmx);
    }
    ASSERT(s_aborto_disparado == 1, "5to PUSH en pila de 16 bytes dispara STACK OVERFLOW");
    ASSERT(strstr(s_msg_aborto, "STACK OVERFLOW") != NULL, "Mensaje confirma STACK OVERFLOW");

    // Eliminar archivo temporal
    remove(ruta_temp);
    set_abortar_handler(NULL);
}

// ----------------------------------------------------------------------------
// 2. Tests de conversion y operaciones entre 16 y 32 bits (y 8 bits)
// ----------------------------------------------------------------------------
void correr_tests_refuerzo_16_a_32_bits(void) {
    Vmx vmx;
    int32_t val;
    int32_t op_mem;
    int32_t val_leido;

    printf("\n--- TESTS REFUERZO: Conversiones entre 16 y 32 Bits ---\n");

    inicializar_vmx(&vmx, 16, 0, 0);

    // 2.1 Extension de signo exhaustiva de 16 a 32 bits
    ASSERT_EQUAL(extender_signo_16_a_32(0x0000), 0, "Extension de 0x0000 da 0");
    ASSERT_EQUAL(extender_signo_16_a_32(0x0001), 1, "Extension de 0x0001 da 1");
    ASSERT_EQUAL(extender_signo_16_a_32(0x7FFF), 32767, "Extension de 0x7FFF (max positivo 16b) da 32767");
    ASSERT_EQUAL(extender_signo_16_a_32(0x8000), -32768, "Extension de 0x8000 (min negativo 16b) da -32768");
    ASSERT_EQUAL(extender_signo_16_a_32(0xFFFF), -1, "Extension de 0xFFFF (-1 16b) da -1 (0xFFFFFFFF)");
    ASSERT_EQUAL(extender_signo_16_a_32(0xFFFE), -2, "Extension de 0xFFFE (-2 16b) da -2");
    ASSERT_EQUAL(extender_signo_16_a_32(0xAAAA), (int32_t)0xFFFFAAAA, "Extension de 0xAAAA con bit 15 en 1 preserva signo");
    ASSERT_EQUAL(extender_signo_16_a_32(0x5555), 0x00005555, "Extension de 0x5555 con bit 15 en 0 preserva ceros");

    // 2.2 Modificador de memoria w[...] (16 bits)
    // Asignar Data Segment en entrada 1: base=200, tamano=1000
    vmx.memoria.tabla_segmentos[1] = (200 << 16) | 1000;
    vmx.registros[DS] = 0x00010000;

    // Escritura de valor 32 bits truncado a 16 bits: 0x1234ABCD en w[DS+10]
    // Codigo modificador word = 010 (2 en bits 5..7), offset 10 en bits 8..23, DS en bits 0..4
    op_mem = (10 << 8) | (2 << 5) | DS;
    set_valor(&vmx, TIPO_MEMORIA, op_mem, 0x1234ABCD);

    // Verificar en memoria fisica: solo se escribieron los 2 bytes bajos (0xAB y 0xCD)
    ASSERT_EQUAL((int)vmx.memoria.mem_principal[210], 0xAB, "Byte 0 en memoria contiene 0xAB (truncado 16 bits)");
    ASSERT_EQUAL((int)vmx.memoria.mem_principal[211], 0xCD, "Byte 1 en memoria contiene 0xCD (truncado 16 bits)");

    // Lectura de 16 bits negativos con extension de signo: 0xABCD -> 0xFFFFABCD (-21555)
    val = get_valor(&vmx, TIPO_MEMORIA, op_mem);
    ASSERT_EQUAL(val, (int32_t)0xFFFFABCD, "Lectura de w[...] con bit 15 en 1 extiende signo a 0xFFFFABCD");

    // Lectura de 16 bits positivos con extension de signo: 0x1234 -> 0x00001234 (4660)
    set_valor(&vmx, TIPO_MEMORIA, op_mem, 0x00001234);
    val = get_valor(&vmx, TIPO_MEMORIA, op_mem);
    ASSERT_EQUAL(val, 4660, "Lectura de w[...] con bit 15 en 0 extiende con ceros a 4660");

    // 2.3 Modificador de memoria b[...] (8 bits)
    // Codigo modificador byte = 001 (1 en bits 5..7), offset 20 en bits 8..23, DS en bits 0..4
    op_mem = (20 << 8) | (1 << 5) | DS;
    set_valor(&vmx, TIPO_MEMORIA, op_mem, 0x123456FE);

    // Verificar en memoria fisica: solo se escribio 0xFE
    ASSERT_EQUAL((int)vmx.memoria.mem_principal[220], 0xFE, "Byte en memoria contiene 0xFE");

    // Lectura de 8 bits negativos: 0xFE -> -2 (0xFFFFFFFE)
    val = get_valor(&vmx, TIPO_MEMORIA, op_mem);
    ASSERT_EQUAL(val, -2, "Lectura de b[...] con 0xFE extiende signo a -2");

    // Lectura de 8 bits positivos: 0x7F -> 127
    set_valor(&vmx, TIPO_MEMORIA, op_mem, 0x7F);
    val = get_valor(&vmx, TIPO_MEMORIA, op_mem);
    ASSERT_EQUAL(val, 127, "Lectura de b[...] con 0x7F extiende con ceros a 127");

    // 2.4 Sectores de registro (AX, AH, AL) sin invadir los demas bytes de EAX
    vmx.registros[EAX] = 0x11223344;

    // Lectura sector AL (sector 01)
    val_leido = get_valor(&vmx, TIPO_REGISTRO, (1 << 5) | EAX);
    ASSERT_EQUAL(val_leido, 0x44, "Lectura AL de 0x11223344 devuelve 0x44 (68)");

    // Lectura sector AH (sector 10)
    val_leido = get_valor(&vmx, TIPO_REGISTRO, (2 << 5) | EAX);
    ASSERT_EQUAL(val_leido, 0x33, "Lectura AH de 0x11223344 devuelve 0x33 (51)");

    // Lectura sector AX (sector 11)
    val_leido = get_valor(&vmx, TIPO_REGISTRO, (3 << 5) | EAX);
    ASSERT_EQUAL(val_leido, 0x3344, "Lectura AX de 0x11223344 devuelve 0x3344 (13124)");

    // Lectura sector AX con signo negativo (0x8050 -> 0xFFFF8050)
    vmx.registros[EAX] = 0x12348050;
    val_leido = get_valor(&vmx, TIPO_REGISTRO, (3 << 5) | EAX);
    ASSERT_EQUAL(val_leido, (int32_t)0xFFFF8050, "Lectura AX negativo extiende signo a 0xFFFF8050");

    // Escritura en AL preservando bytes 1, 2, 3
    vmx.registros[EAX] = 0x11223344;
    set_valor(&vmx, TIPO_REGISTRO, (1 << 5) | EAX, 0x99);
    ASSERT_EQUAL(vmx.registros[EAX], (int32_t)0x11223399, "Escritura en AL modifica solo el byte 0 de EAX");

    // Escritura en AH preservando bytes 0, 2, 3
    set_valor(&vmx, TIPO_REGISTRO, (2 << 5) | EAX, 0xAA);
    ASSERT_EQUAL(vmx.registros[EAX], (int32_t)0x1122AA99, "Escritura en AH modifica solo el byte 1 de EAX");

    // Escritura en AX preservando bytes 2 y 3
    set_valor(&vmx, TIPO_REGISTRO, (3 << 5) | EAX, 0x5566);
    ASSERT_EQUAL(vmx.registros[EAX], (int32_t)0x11225566, "Escritura en AX modifica solo los 16 bits bajos de EAX");

    // 2.5 Carga combinada con LDL y LDH para formar numeros de 32 bits extremos
    // Formar INT32_MAX: LDL 0xFFFF, LDH 0x7FFF -> 0x7FFFFFFF (2147483647)
    vmx.registros[EBX] = 0;
    combinar_mitad(&vmx, TIPO_REGISTRO, EBX, 0xFFFF, 0); // LDL
    combinar_mitad(&vmx, TIPO_REGISTRO, EBX, 0x7FFF, 1); // LDH
    ASSERT_EQUAL(vmx.registros[EBX], 2147483647, "LDL + LDH combinan correctamente para formar INT32_MAX (2147483647)");

    // Formar INT32_MIN: LDL 0x0000, LDH 0x8000 -> 0x80000000 (-2147483648)
    combinar_mitad(&vmx, TIPO_REGISTRO, EBX, 0x0000, 0);
    combinar_mitad(&vmx, TIPO_REGISTRO, EBX, 0x8000, 1);
    ASSERT_EQUAL(vmx.registros[EBX], (int32_t)0x80000000, "LDL + LDH combinan correctamente para formar INT32_MIN (-2147483648)");
}

// ----------------------------------------------------------------------------
// 3. Tests de refuerzo de manejo de cadenas y arboles
// ----------------------------------------------------------------------------
void correr_tests_refuerzo_cadenas_y_arboles(void) {
    Vmx vmx;
    char ruta_bin[260];
    InfoParamSegment ps_vacia;

    printf("\n--- TESTS REFUERZO: Cadenas y Arboles Avanzados ---\n");

    ps_vacia.tamano = 0;
    ps_vacia.argc = 0;
    ps_vacia.dir_argv = -1;

    // 3.1 Binario completo 08_comparacion_cadenas.vmx:
    // Compara CADENA_A vs CADENA_B (iguales -> EBX=0)
    // Compara CADENA_A vs CADENA_C (distintas -> ECX!=0)
    inicializar_vmx(&vmx, 16, 0, 0);
    resolver_ruta_refuerzo("vmx/p2_completos/08_comparacion_cadenas.vmx", ruta_bin);
    cargar_programa(&vmx, ruta_bin, ps_vacia.tamano);
    if (vmx.registros[SS] != -1) {
        inicializar_subrutina_principal(&vmx, ps_vacia);
    }
    ejecutar_vmx(&vmx);

    assert_registro(&vmx, EBX, 0, "08_comparacion_cadenas.vmx: Cadenas iguales retornan EBX = 0");
    ASSERT(vmx.registros[ECX] != 0, "08_comparacion_cadenas.vmx: Cadenas distintas retornan ECX != 0");

    // 3.2 Binario completo 04_arbol_binario_max_min.vmx:
    // Arbol Binario en Extra Segment:
    // Minimo: 15 (en EBX)
    // Maximo: 95 (en ECX)
    // Conteo recursivo total de nodos: 7 (en EDX)
    inicializar_vmx(&vmx, 16, 0, 0);
    resolver_ruta_refuerzo("vmx/p2_avanzados/04_arbol_binario_max_min.vmx", ruta_bin);
    cargar_programa(&vmx, ruta_bin, ps_vacia.tamano);
    if (vmx.registros[SS] != -1) {
        inicializar_subrutina_principal(&vmx, ps_vacia);
    }
    ejecutar_vmx(&vmx);

    assert_registro(&vmx, EBX, 15, "04_arbol_binario_max_min.vmx: Minimo del BST es 15");
    assert_registro(&vmx, EFX, 95, "04_arbol_binario_max_min.vmx: Maximo del BST es 95");
    assert_registro(&vmx, EDX, 7, "04_arbol_binario_max_min.vmx: Conteo recursivo total de nodos es 7");
}

// ----------------------------------------------------------------------------
// 4. Verificacion de validaciones de la especificacion de la Parte 2
// ----------------------------------------------------------------------------
void correr_tests_refuerzo_validaciones_especificacion(void) {
    Vmx vmx;
    char ruta_bin[260];
    InfoParamSegment ps_vacia;

    printf("\n--- TESTS REFUERZO: Validaciones de Especificacion Parte 2 ---\n");

    ps_vacia.tamano = 0;
    ps_vacia.argc = 0;
    ps_vacia.dir_argv = -1;

    set_abortar_handler(mock_abortar_refuerzo);

    // 4.1 Verificacion de compatibilidad con cabecera V1:
    // Binario 01_add_basico.vmx tiene cabecera V1
    inicializar_vmx(&vmx, 16, 0, 0);
    resolver_ruta_refuerzo("vmx/aritmetica/01_add_basico.vmx", ruta_bin);
    cargar_programa(&vmx, ruta_bin, 0);

    ASSERT_EQUAL(vmx.registros[CS], 0x00000000, "V1: Registro CS es 0x00000000");
    ASSERT_EQUAL(vmx.registros[DS], 0x00010000, "V1: Registro DS es 0x00010000");
    ASSERT_EQUAL(vmx.registros[IP], 0x00000000, "V1: Registro IP inicializado en CS");
    ASSERT_EQUAL(vmx.registros[ES], -1, "V1: Registro ES fijado en -1");
    ASSERT_EQUAL(vmx.registros[SS], -1, "V1: Registro SS fijado en -1");
    ASSERT_EQUAL(vmx.registros[KS], -1, "V1: Registro KS fijado en -1");
    ASSERT_EQUAL(vmx.registros[PS], -1, "V1: Registro PS fijado en -1");
    ASSERT_EQUAL(vmx.memoria.tabla_segmentos[2], 0xFFFFFFFF, "V1: Entrada 2 de tabla es invalida");

    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 20, "V1: Ejecucion completa exitosa deja EAX = 20");

    // 4.2 Validacion de subrutina principal en la pila (Stack Segment):
    // La especificacion pide que al iniciar la VM con V2:
    // SP apunta a RET (-1)
    // SP+4 apunta a argc
    // SP+8 apunta a *argv
    inicializar_vmx(&vmx, 16, 0, 0);
    crear_segmentos(&vmx.memoria, 0, 0, 64, 64, 0, 128); // SS en entrada 2, tam 128
    inicializar_subrutina_principal(&vmx, ps_vacia);

    int32_t val_sp0 = sacar_pila(&vmx);
    ASSERT_EQUAL(val_sp0, -1, "Subrutina principal: Tope de pila (SP) contiene RET = -1");
    int32_t val_sp4 = sacar_pila(&vmx);
    ASSERT_EQUAL(val_sp4, 0, "Subrutina principal: Segundo elemento (SP+4) contiene argc = 0");
    int32_t val_sp8 = sacar_pila(&vmx);
    ASSERT_EQUAL(val_sp8, -1, "Subrutina principal: Tercer elemento (SP+8) contiene *argv = -1");

    // 4.3 Inicializacion con parametros (-p prueba 123)
    char args[2][100];
    strcpy(args[0], "prueba");
    strcpy(args[1], "123");
    InfoParamSegment info_p = construir_param_segment(&vmx, "programa.vmx", args, 2);

    ASSERT(info_p.tamano > 0, "Param Segment construido con tamano mayor a cero");
    ASSERT_EQUAL(info_p.argc, 3, "Param Segment tiene argc = 3 (nombre + 2 argumentos)");
    ASSERT(info_p.dir_argv > 0, "Puntero dir_argv apunta al arreglo de punteros");

    inicializar_vmx(&vmx, 16, 0, 0);
    crear_segmentos(&vmx.memoria, info_p.tamano, 0, 64, 64, 0, 128);
    inicializar_subrutina_principal(&vmx, info_p);

    val_sp0 = sacar_pila(&vmx);
    ASSERT_EQUAL(val_sp0, -1, "Con parametros: SP contiene RET = -1");
    val_sp4 = sacar_pila(&vmx);
    ASSERT_EQUAL(val_sp4, 3, "Con parametros: SP+4 contiene argc = 3");
    val_sp8 = sacar_pila(&vmx);
    ASSERT_EQUAL(val_sp8, info_p.dir_argv, "Con parametros: SP+8 contiene *argv de Param Segment");

    set_abortar_handler(NULL);
}

// ----------------------------------------------------------------------------
// Suite general de tests de refuerzo
// ----------------------------------------------------------------------------
void correr_tests_refuerzo(void) {
    correr_tests_refuerzo_memoria_bordes();
    correr_tests_refuerzo_16_a_32_bits();
    correr_tests_refuerzo_cadenas_y_arboles();
    correr_tests_refuerzo_validaciones_especificacion();
}
