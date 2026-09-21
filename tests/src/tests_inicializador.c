#include "tests_helpers.h"
#include <stdio.h>
#include <string.h>

void correr_tests_inicializador(void) {
    Vmx vmx;
    int i;
    int ok;
    int tam_cod;
    int base_datos;
    int tam_datos;
    char ruta_vmx[100];
    FILE *arch_test;

    printf("\n--- TESTS: Inicializador y Estado Base ---\n");

    // 1. Test inicializar_vmx
    inicializar_vmx(&vmx, 0, 0);

    ok = 1;
    for (i = 0; i < 32; i++) {
        if (vmx.registros[i] != 0) {
            ok = 0;
            break;
        }
    }
    ASSERT(ok, "inicializar_vmx: Los 32 registros quedan en 0");

    ok = 1;
    for (i = 0; i < 16384; i++) {
        if (vmx.memoria.mem_principal[i] != 0) {
            ok = 0;
            break;
        }
    }
    ASSERT(ok, "inicializar_vmx: Los 16 KiB de memoria principal quedan en 0");

    ok = 1;
    for (i = 0; i < 8; i++) {
        if (vmx.memoria.tabla_segmentos[i] != 0xFFFFFFFF) {
            ok = 0;
            break;
        }
    }
    ASSERT(ok, "inicializar_vmx: Las 8 entradas de tabla_segmentos quedan en 0xFFFFFFFF");
    ASSERT(vmx.abortar != NULL, "inicializar_vmx: Metodo abortar queda enlazado");

    // 2. Test cargar_programa con binario real
    strcpy(ruta_vmx, "tests/vmx/saltos/jmp_fin.vmx");
    arch_test = fopen(ruta_vmx, "rb");
    if (arch_test == NULL) {
        // Si se ejecuta desde la carpeta tests/
        strcpy(ruta_vmx, "vmx/saltos/jmp_fin.vmx");
    } else {
        fclose(arch_test);
    }

    inicializar_vmx(&vmx, 0, 0);
    cargar_programa(&vmx, ruta_vmx);

    ASSERT_EQUAL(vmx.registros[CS], 0x00000000, "cargar_programa: Registro CS apunta a 0x00000000");
    ASSERT_EQUAL(vmx.registros[DS], 0x00010000, "cargar_programa: Registro DS apunta a 0x00010000");
    ASSERT_EQUAL(vmx.registros[IP], vmx.registros[CS], "cargar_programa: Registro IP inicializado igual a CS");

    tam_cod = (int)(vmx.memoria.tabla_segmentos[0] & 0xFFFF);
    ASSERT(tam_cod > 0, "cargar_programa: Entrada 0 de tabla de segmentos tiene tamano > 0");
    ASSERT_EQUAL((int)((vmx.memoria.tabla_segmentos[0] >> 16) & 0xFFFF), 0, "cargar_programa: Entrada 0 tiene base 0");

    base_datos = (int)((vmx.memoria.tabla_segmentos[1] >> 16) & 0xFFFF);
    ASSERT_EQUAL(base_datos, tam_cod, "cargar_programa: Base del segmento de datos coincide con tamano de codigo");

    tam_datos = (int)(vmx.memoria.tabla_segmentos[1] & 0xFFFF);
    ASSERT_EQUAL(tam_datos, 16384 - tam_cod, "cargar_programa: Tamano de datos es 16384 - tamano_codigo");

    // 3. Test ip_valido
    ASSERT_EQUAL(ip_valido(&vmx), 1, "ip_valido: IP inicial (0) es valido");

    vmx.registros[IP] = -1;
    ASSERT_EQUAL(ip_valido(&vmx), 0, "ip_valido: IP en -1 (STOP) es detectado como fin");

    vmx.registros[IP] = 0x00010000; // Segmento 1 (datos)
    ASSERT_EQUAL(ip_valido(&vmx), 0, "ip_valido: IP en segmento 1 (datos) no es valido para ejecucion");

    vmx.registros[IP] = tam_cod; // Fuera del limite del codigo
    ASSERT_EQUAL(ip_valido(&vmx), 0, "ip_valido: IP fuera del tamano de codigo no es valido");

    // 4. Test con el programa completo de conteo de bits
    strcpy(ruta_vmx, "tests/vmx/general/conteo_bits.vmx");
    arch_test = fopen(ruta_vmx, "rb");
    if (arch_test == NULL) {
        strcpy(ruta_vmx, "vmx/general/conteo_bits.vmx");
    } else {
        fclose(arch_test);
    }

    inicializar_vmx(&vmx, 0, 0);
    cargar_programa(&vmx, ruta_vmx);
    tam_cod = (int)(vmx.memoria.tabla_segmentos[0] & 0xFFFF);
    ASSERT(tam_cod > 0, "cargar_programa: Programa conteo_bits.vmx cargado correctamente con tamano > 0");
    ASSERT_EQUAL(ip_valido(&vmx), 1, "ip_valido: Programa conteo_bits.vmx listo para ciclo de ejecucion");
}
