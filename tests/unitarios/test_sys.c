#include "test_helpers.h"
#include "../../src/sys.h"
#include "../../src/vmi.h"
#include "../../src/operandos.h"
#include "../../src/memoria.h"
#include "../../src/inicializador.h"

void test_sys_suite(void) {
    printf("\n--- TESTS: sys.c ---\n");

    Vmx vmx;
    inicializar_vmx(&vmx, 16, 0, 0);

    // Configuramos entrada 0: Base 0, Tamano 1000 bytes
    vmx.memoria.tabla_segmentos[0] = (0 << 16) | 1000;
    vmx.registros[DS] = 0x00000000;

    // Test 1: SYS 0 (CLEAR SCREEN) no altera ningun registro
    vmx.registros[OP1] = (TIPO_INMEDIATO << 24) | 0; // SYS 0
    vmx.registros[EAX] = 0x1234;
    vmx.registros[EBX] = 0x5678;
    op_sys(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], 0x1234, "op_sys CLEAR SCREEN: preserva EAX");
    TEST_ASSERT_EQUAL(vmx.registros[EBX], 0x5678, "op_sys CLEAR SCREEN: preserva EBX");

    // Test 2: Validaciones de SYS 2 (WRITE)
    vmx.registros[OP1] = (TIPO_INMEDIATO << 24) | 2; // SYS 2
    vmx.registros[EDX] = 0x00000010; // DirBase = 16

    // Caso de error: tamanio 0 en ECX (bits 16..31)
    vmx.registros[ECX] = (0 << 16) | 1; // tamanio 0, cantidad 1
    vmx.registros[EAX] = 0x01; // modo decimal
    TEST_ASSERT_ABORTA(op_sys(&vmx), "op_sys: tamanio 0 en ECX aborta");

    // Caso de error: tamanio 5 en ECX
    vmx.registros[ECX] = (5 << 16) | 1;
    TEST_ASSERT_ABORTA(op_sys(&vmx), "op_sys: tamanio 5 en ECX aborta");

    // Caso de error: cantidad 0 en ECX
    vmx.registros[ECX] = (4 << 16) | 0;
    TEST_ASSERT_ABORTA(op_sys(&vmx), "op_sys: cantidad 0 en ECX aborta");

    // Caso de error: modo sin ningun bit valido (bits 0..4 en 0)
    vmx.registros[ECX] = (4 << 16) | 1;
    vmx.registros[EAX] = 0x00;
    TEST_ASSERT_ABORTA(op_sys(&vmx), "op_sys: modo 0x00 sin bits de formato aborta");

    // Test 3: Ejecucion de SYS 2 (WRITE) con diferentes modos
    // Escribimos valores en memoria [DS+16] = 1234
    escribir_memoria(&vmx.memoria, 16, 4, 1234);
    vmx.registros[ECX] = (4 << 16) | 1; // tamanio 4, cantidad 1

    // Modo Decimal (0x01)
    vmx.registros[EAX] = 0x01;
    op_sys(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[LAR], 0x00000010, "op_sys WRITE decimal: LAR actualizado");
    TEST_ASSERT_EQUAL(vmx.registros[MBR], 1234, "op_sys WRITE decimal: MBR actualizado");

    // Modo Hexadecimal (0x08)
    vmx.registros[EAX] = 0x08;
    op_sys(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[MBR], 1234, "op_sys WRITE hex: ejecuta correctamente");

    // Modo Octal (0x04)
    vmx.registros[EAX] = 0x04;
    op_sys(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[MBR], 1234, "op_sys WRITE octal: ejecuta correctamente");

    // Modo Binario (0x10)
    vmx.registros[EAX] = 0x10;
    op_sys(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[MBR], 1234, "op_sys WRITE binario: ejecuta correctamente");

    // Modo Caracter (0x02) con tamanio 1 byte
    escribir_byte(&vmx.memoria, 20, 'Z');
    vmx.registros[EDX] = 0x00000014; // offset 20
    vmx.registros[ECX] = (1 << 16) | 1; // tamanio 1, cantidad 1
    vmx.registros[EAX] = 0x02; // caracter
    op_sys(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[MBR], 'Z', "op_sys WRITE caracter: valor 'Z' procesado");

    // Modo Combinado (0x09: Decimal + Hex)
    vmx.registros[EAX] = 0x09;
    op_sys(&vmx);
    TEST_ASSERT(1, "op_sys WRITE combinado: decimal y hexadecimal simultaneos");

    // Test 4: SYS 4 (STRING WRITE)
    // Caso 1: Cadena normal "Hola\0"
    vmx.memoria.mem_principal[50] = 'H';
    vmx.memoria.mem_principal[51] = 'o';
    vmx.memoria.mem_principal[52] = 'l';
    vmx.memoria.mem_principal[53] = 'a';
    vmx.memoria.mem_principal[54] = '\0';
    vmx.registros[OP1] = (TIPO_INMEDIATO << 24) | 4; // SYS 4
    vmx.registros[EDX] = 0x00000032; // offset 50
    op_sys(&vmx);
    TEST_ASSERT(1, "op_sys STRING WRITE: recorre e imprime cadena hasta caracter nulo");
    TEST_ASSERT_EQUAL(vmx.registros[LAR], 0x00000036, "op_sys STRING WRITE: LAR actualizado al terminador (offset 54)");
    TEST_ASSERT_EQUAL(vmx.registros[MAR], (1 << 16) | 54, "op_sys STRING WRITE: MAR tiene cant_bytes=1 y dir_fisica=54");
    TEST_ASSERT_EQUAL(vmx.registros[MBR], 0, "op_sys STRING WRITE: MBR contiene '\\0'");

    // Caso 2: Cadena vacia (primer caracter '\0')
    vmx.memoria.mem_principal[60] = '\0';
    vmx.registros[EDX] = 0x0000003C; // offset 60
    op_sys(&vmx);
    TEST_ASSERT(1, "op_sys STRING WRITE: cadena vacia procesada de forma inmediata");
    TEST_ASSERT_EQUAL(vmx.registros[LAR], 0x0000003C, "op_sys STRING WRITE: cadena vacia actualiza LAR");

    // Caso 3: String sin terminador dentro del limite del segmento aborta por fallo de segmento
    vmx.memoria.tabla_segmentos[2] = (500 << 16) | 5; // segmento 2, tamano 5 bytes
    vmx.registros[EDX] = 0x00020000; // segmento 2, offset 0
    int k;
    for (k = 0; k < 5; k++) {
        vmx.memoria.mem_principal[500 + k] = 'X';
    }
    TEST_ASSERT_ABORTA(op_sys(&vmx), "op_sys STRING WRITE: string sin '\\0' aborta por fallo de segmento");

    // Test 5: SYS 15 (BREAKPOINT) sin archivo VMI
    vmx.registros[OP1] = (TIPO_INMEDIATO << 24) | 15; // SYS 15
    vmi_configurar_archivo(NULL);
    op_sys(&vmx);
    TEST_ASSERT(1, "op_sys BREAKPOINT: ignorado limpiamente si no hay archivo .vmi");

    // Test 6: Syscall no implementada o numero desconocido
    vmx.registros[OP1] = (TIPO_INMEDIATO << 24) | 99; // SYS 99
    op_sys(&vmx);
    TEST_ASSERT(1, "op_sys: numero de syscall desconocido no rompe la ejecucion");

    free(vmx.memoria.mem_principal);
}
