#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../../src/vmx.h"
#include "../../src/inicializador.h"
#include "../../src/param_segment.h"
#include "../../src/memoria.h"
#include "tests_helpers.h"

int tests_ejecutados = 0;
int tests_aprobados = 0;
int tests_fallados = 0;

static void resolver_ruta_amigo(const char relativa[], char ruta_final[]) {
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

static int32_t leer_ds_long(Vmx *vmx, int offset) {
    int32_t dir_logica = vmx->registros[DS] + (offset & 0xFFFF);
    return get_valor_memoria(&vmx->memoria, dir_logica, 4);
}

static void ejecutar_y_analizar_amigo(void) {
    Vmx vmx;
    InfoParamSegment ps_vacia = {0};
    char ruta_bin[256];
    
    printf("\n--- EJECUTANDO CODIGO DEL AMIGO (EJERCICIO 11) ---\n");
    inicializar_vmx(&vmx, 16, 0, 0);
    resolver_ruta_amigo("vmx/guia_de_ejercicios/ej11/main_amigo_ej11.vmx", ruta_bin);
    
    cargar_programa(&vmx, ruta_bin, ps_vacia.tamano);
    if (vmx.registros[SS] != -1) {
        inicializar_subrutina_principal(&vmx, ps_vacia);
    }
    
    ejecutar_vmx(&vmx);
    
    int val_eax = vmx.registros[EAX];
    int val_ecx = vmx.registros[ECX];
    
    printf("Resultado retornado en EAX: %d (esperado: 5)\n", val_eax);
    printf("Resultado retornado en ECX: %d (esperado: 5)\n", val_ecx);
    
    ASSERT_EQUAL(val_eax, 5, "Valor retornado en EAX == 5");
    ASSERT_EQUAL(val_ecx, 5, "Valor retornado en ECX == 5");
    
    uint32_t ds_reg = (uint32_t)vmx.registros[DS];
    
    // =========================================================================
    // 1. VERIFICACION DE LA LISTA PRINCIPAL
    // Diagrama esperado: Celda 1 -> Celda 2 -> Celda 4 -> NULL (Celda 3 eliminada)
    // =========================================================================
    uint32_t p_lista = (uint32_t)leer_ds_long(&vmx, 0);
    ASSERT_EQUAL(p_lista, ds_reg + 16, "Lista principal inicia en Celda 1 (DS+16)");
    
    uint32_t c1_arbol = (uint32_t)leer_ds_long(&vmx, 16);
    uint32_t c1_sig   = (uint32_t)leer_ds_long(&vmx, 20);
    ASSERT_EQUAL(c1_arbol, ds_reg + 64, "Celda 1 apunta a Raiz 1 (DS+64)");
    ASSERT_EQUAL(c1_sig,   ds_reg + 24, "Celda 1 sig apunta a Celda 2 (DS+24)");
    
    uint32_t c2_arbol = (uint32_t)leer_ds_long(&vmx, 24);
    uint32_t c2_sig   = (uint32_t)leer_ds_long(&vmx, 28);
    ASSERT_EQUAL(c2_arbol, ds_reg + 180, "Celda 2 apunta a Raiz 5 (DS+180)");
    ASSERT_EQUAL(c2_sig,   ds_reg + 40,  "Celda 2 sig apunta a Celda 4 (DS+40) [Celda 3 desvinculada]");
    
    uint32_t c4_arbol = (uint32_t)leer_ds_long(&vmx, 40);
    uint32_t c4_sig   = (uint32_t)leer_ds_long(&vmx, 44);
    ASSERT_EQUAL(c4_arbol, ds_reg + 280, "Celda 4 apunta a Raiz 2 (DS+280)");
    ASSERT_EQUAL(c4_sig,   0xFFFFFFFF,   "Celda 4 sig es NULL (fin de lista)");
    
    // =========================================================================
    // 2. VERIFICACION DE ARBOL 1 (Raiz 1 en DS+64)
    // Raiz 1 -> hijo 2 (con nieto 1) y hijo 3 (con nieto 5, nieto 4 eliminado)
    // =========================================================================
    int val_r1 = leer_ds_long(&vmx, 64);
    ASSERT_EQUAL(val_r1, 1, "Arbol 1: Raiz valor es 1");
    uint32_t r1_hijos = (uint32_t)leer_ds_long(&vmx, 68);
    ASSERT_EQUAL(r1_hijos, ds_reg + 80, "Arbol 1: Raiz apunta a celda de hijos C1_1 (DS+80)");
    
    // C1_1 -> arbol N1_2 (val 2), sig -> C1_2 (DS+88)
    uint32_t c1_1_arbol = (uint32_t)leer_ds_long(&vmx, 80);
    uint32_t c1_1_sig   = (uint32_t)leer_ds_long(&vmx, 84);
    ASSERT_EQUAL(c1_1_arbol, ds_reg + 96, "Arbol 1: Primer hijo es N1_2 (DS+96)");
    ASSERT_EQUAL(c1_1_sig,   ds_reg + 88, "Arbol 1: Siguiente hijo es C1_2 (DS+88)");
    ASSERT_EQUAL(leer_ds_long(&vmx, 96), 2, "Arbol 1: Nodo hijo 1 valor == 2");
    
    // N1_2 -> nieto 1
    uint32_t n1_2_hijos = (uint32_t)leer_ds_long(&vmx, 100);
    uint32_t c1_2_1_arbol = (uint32_t)leer_ds_long(&vmx, n1_2_hijos - ds_reg);
    ASSERT_EQUAL(leer_ds_long(&vmx, c1_2_1_arbol - ds_reg), 1, "Arbol 1: Nieto de hijo 2 valor == 1");
    
    // C1_2 -> arbol N1_3 (val 3)
    uint32_t c1_2_arbol = (uint32_t)leer_ds_long(&vmx, 88);
    ASSERT_EQUAL(c1_2_arbol, ds_reg + 128, "Arbol 1: Segundo hijo es N1_3 (DS+128)");
    ASSERT_EQUAL(leer_ds_long(&vmx, 128), 3, "Arbol 1: Nodo hijo 2 valor == 3");
    
    // N1_3 (hijo 3) tenia nietos 4 y 5. Al eliminar 4, solo debe quedar nieto 5
    uint32_t n1_3_hijos = (uint32_t)leer_ds_long(&vmx, 132);
    ASSERT(n1_3_hijos != 0xFFFFFFFF, "Arbol 1: Hijo 3 tiene celda de hijos");
    if (n1_3_hijos != 0xFFFFFFFF) {
        uint32_t c_nieto_arbol = (uint32_t)leer_ds_long(&vmx, n1_3_hijos - ds_reg);
        uint32_t c_nieto_sig   = (uint32_t)leer_ds_long(&vmx, (n1_3_hijos - ds_reg) + 4);
        int val_nieto = (c_nieto_arbol != 0xFFFFFFFF) ? leer_ds_long(&vmx, c_nieto_arbol - ds_reg) : -1;
        ASSERT_EQUAL(val_nieto, 5, "Arbol 1: Hijo 3 ahora solo tiene nieto 5 (hoja 4 eliminada)");
        ASSERT_EQUAL(c_nieto_sig, 0xFFFFFFFF, "Arbol 1: Hijo 3 no tiene mas hijos (sig == NULL)");
    }
    
    // =========================================================================
    // 3. VERIFICACION DE ARBOL 2 (Raiz 5 en DS+180)
    // Raiz 5 -> hijo 2 (hoja) y hijo 1 (promovido tras eliminar 4)
    // =========================================================================
    int val_r2 = leer_ds_long(&vmx, 180);
    ASSERT_EQUAL(val_r2, 5, "Arbol 2: Raiz valor es 5");
    uint32_t r2_hijos = (uint32_t)leer_ds_long(&vmx, 184);
    ASSERT_EQUAL(r2_hijos, ds_reg + 192, "Arbol 2: Raiz apunta a celda C2_1 (DS+192)");
    
    // C2_1 -> nodo val 2, sig -> C2_2 (DS+212)
    uint32_t c2_1_arb = (uint32_t)leer_ds_long(&vmx, 192);
    uint32_t c2_1_s   = (uint32_t)leer_ds_long(&vmx, 196);
    ASSERT_EQUAL(leer_ds_long(&vmx, c2_1_arb - ds_reg), 2, "Arbol 2: Primer hijo valor == 2");
    ASSERT_EQUAL(c2_1_s, ds_reg + 212, "Arbol 2: Celda siguiente es C2_2 (DS+212)");
    
    // C2_2 -> nodo val 1 (promovido), sig -> NULL
    uint32_t c2_2_arb = (uint32_t)leer_ds_long(&vmx, 212);
    uint32_t c2_2_s   = (uint32_t)leer_ds_long(&vmx, 216);
    ASSERT_EQUAL(c2_2_arb, ds_reg + 240, "Arbol 2: C2_2 ahora apunta al nodo 1 promovido (DS+240)");
    ASSERT_EQUAL(leer_ds_long(&vmx, c2_2_arb - ds_reg), 1, "Arbol 2: Segundo hijo es el nodo 1");
    ASSERT_EQUAL(c2_2_s, 0xFFFFFFFF, "Arbol 2: C2_2 sig es NULL");
    
    // =========================================================================
    // 4. VERIFICACION DE ARBOL 4 (Raiz 2 en DS+280)
    // Raiz 2 -> hijo 2 y hijo 1 (promovido con hijos 6 y 7). Tercer hijo 4 hoja eliminado.
    // =========================================================================
    int val_r4 = leer_ds_long(&vmx, 280);
    ASSERT_EQUAL(val_r4, 2, "Arbol 4: Raiz valor es 2");
    uint32_t r4_hijos = (uint32_t)leer_ds_long(&vmx, 284);
    ASSERT_EQUAL(r4_hijos, ds_reg + 296, "Arbol 4: Raiz apunta a celda C4_1 (DS+296)");
    
    // Primer hijo de Raiz 2: nodo 2
    uint32_t c4_1_arb = (uint32_t)leer_ds_long(&vmx, 296);
    uint32_t c4_1_s   = (uint32_t)leer_ds_long(&vmx, 300);
    ASSERT_EQUAL(leer_ds_long(&vmx, c4_1_arb - ds_reg), 2, "Arbol 4: Primer hijo valor == 2");
    ASSERT_EQUAL(c4_1_s, ds_reg + 304, "Arbol 4: Celda siguiente es C4_2 (DS+304)");
    
    // Segundo hijo de Raiz 2: nodo 1 promovido (DS+360), y tercer hijo (4 hoja) eliminado
    uint32_t c4_2_arb = (uint32_t)leer_ds_long(&vmx, 304);
    uint32_t c4_2_s   = (uint32_t)leer_ds_long(&vmx, 308);
    ASSERT_EQUAL(c4_2_arb, ds_reg + 360, "Arbol 4: C4_2 apunta a nodo 1 promovido (DS+360)");
    ASSERT_EQUAL(leer_ds_long(&vmx, c4_2_arb - ds_reg), 1, "Arbol 4: Nodo promovido valor == 1");
    ASSERT_EQUAL(c4_2_s, 0xFFFFFFFF, "Arbol 4: C4_2 sig es NULL (tercer hijo 4 hoja eliminado de lista de hijos)");
    
    // Hijos del nodo 1 promovido: deben ser 6 y 7
    uint32_t n1_prom_hijos = (uint32_t)leer_ds_long(&vmx, (c4_2_arb - ds_reg) + 4);
    ASSERT(n1_prom_hijos != 0xFFFFFFFF, "Arbol 4: Nodo 1 promovido tiene hijos (6 y 7)");
    if (n1_prom_hijos != 0xFFFFFFFF) {
        uint32_t c_h6_arb = (uint32_t)leer_ds_long(&vmx, n1_prom_hijos - ds_reg);
        uint32_t c_h6_sig = (uint32_t)leer_ds_long(&vmx, (n1_prom_hijos - ds_reg) + 4);
        ASSERT_EQUAL(leer_ds_long(&vmx, c_h6_arb - ds_reg), 6, "Arbol 4: Primer hijo de 1 promovido es 6");
        ASSERT(c_h6_sig != 0xFFFFFFFF, "Arbol 4: Existe segunda celda de hijos");
        if (c_h6_sig != 0xFFFFFFFF) {
            uint32_t c_h7_arb = (uint32_t)leer_ds_long(&vmx, c_h6_sig - ds_reg);
            uint32_t c_h7_sig = (uint32_t)leer_ds_long(&vmx, (c_h6_sig - ds_reg) + 4);
            ASSERT_EQUAL(leer_ds_long(&vmx, c_h7_arb - ds_reg), 7, "Arbol 4: Segundo hijo de 1 promovido es 7");
            ASSERT_EQUAL(c_h7_sig, 0xFFFFFFFF, "Arbol 4: Fin de hijos del nodo promovido (sig == NULL)");
        }
    }
}

int main(void) {
    printf("==================================================\n");
    printf("  TEST INDEPENDIENTE: VERIFICACION ESTRUCTURAL AMIGO\n");
    printf("==================================================\n");
    
    ejecutar_y_analizar_amigo();
    
    printf("\n==================================================\n");
    printf("RESUMEN DE PRUEBAS AMIGO:\n");
    printf("  Total ejecutadas: %d\n", tests_ejecutados);
    printf("  Aprobadas:        %d\n", tests_aprobados);
    printf("  Falladas:         %d\n", tests_fallados);
    printf("==================================================\n");
    
    return (tests_fallados == 0) ? 0 : 1;
}
