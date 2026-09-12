#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "tests_helpers.h"
#include "../../src/vmx.h"
#include "../../src/inicializador.h"
#include "../../src/errores.h"

// Mecanismo de captura de abortar para pruebas de excepciones
static jmp_buf s_jmp_abort;
static int s_abort_llamado = 0;
static char s_ultimo_mensaje_abort[256] = "";

static void mock_abortar(char mensaje[]) {
    s_abort_llamado = 1;
    if (mensaje != NULL) {
        strncpy(s_ultimo_mensaje_abort, mensaje, sizeof(s_ultimo_mensaje_abort) - 1);
        s_ultimo_mensaje_abort[sizeof(s_ultimo_mensaje_abort) - 1] = '\0';
    }
    longjmp(s_jmp_abort, 1);
}

// Helper para resolver la ruta del archivo .vmx segun desde donde se ejecute el runner
static void resolver_ruta_vmx(const char *relativa, char ruta_final[]) {
    FILE *f = fopen(relativa, "rb");
    if (f != NULL) {
        fclose(f);
        strcpy(ruta_final, relativa);
        return;
    }
    // Si no abre directo, probar anteponiendo "tests/"
    sprintf(ruta_final, "tests/%s", relativa);
    f = fopen(ruta_final, "rb");
    if (f != NULL) {
        fclose(f);
        return;
    }
    // Si aun no, probar anteponiendo "../"
    sprintf(ruta_final, "../%s", relativa);
    f = fopen(ruta_final, "rb");
    if (f != NULL) {
        fclose(f);
        return;
    }
    // Si no se encuentra, dejar la original
    strcpy(ruta_final, relativa);
}

void correr_tests_binarios(void) {
    Vmx vmx;
    char ruta[260];

    printf("\n--- TESTS DE EJECUCION DE BINARIOS REALES (.vmx) ---\n");

    // 1. mov/01_mov_inm.vmx: mov eax, 42
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/mov/01_mov_inm.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 42, "01_mov_inm.vmx asigna EAX = 42");

    // 2. mov/02_mov_reg.vmx: mov eax, 100; mov ebx, eax
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/mov/02_mov_reg.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 100, "02_mov_reg.vmx mantiene EAX = 100");
    assert_registro(&vmx, EBX, 100, "02_mov_reg.vmx copia a EBX = 100");

    // 3. mov/03_mov_mem.vmx: mov [0], 1234; mov eax, [0]
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/mov/03_mov_mem.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 1234, "03_mov_mem.vmx lee desde memoria EAX = 1234");

    // 4. mov/04_swap_reg.vmx: mov eax, 11; mov ebx, 22; swap eax, ebx
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/mov/04_swap_reg.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 22, "04_swap_reg.vmx intercambia dejando EAX = 22");
    assert_registro(&vmx, EBX, 11, "04_swap_reg.vmx intercambia dejando EBX = 11");

    // 5. aritmetica/01_add_basico.vmx: mov eax, 15; add eax, 5
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/aritmetica/01_add_basico.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 20, "01_add_basico.vmx suma dejando EAX = 20");

    // 6. aritmetica/03_sub_basico.vmx: mov eax, 30; sub eax, 10
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/aritmetica/03_sub_basico.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 20, "03_sub_basico.vmx resta dejando EAX = 20");

    // 7. aritmetica/05_mul_basico.vmx: mov eax, 6; mul eax, 7
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/aritmetica/05_mul_basico.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 42, "05_mul_basico.vmx multiplica dejando EAX = 42");

    // 8. aritmetica/07_div_resto.vmx: mov eax, 23; div eax, 5
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/aritmetica/07_div_resto.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 4, "07_div_resto.vmx guarda cociente en EAX = 4");
    assert_registro(&vmx, AC, 3, "07_div_resto.vmx guarda resto en AC = 3");

    // 9. aritmetica/08_shl_basico.vmx: mov eax, 1; shl eax, 4
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/aritmetica/08_shl_basico.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 16, "08_shl_basico.vmx desplaza a izquierda EAX = 16");

    // 10. aritmetica/09_shr_basico.vmx: mov eax, 32; shr eax, 2
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/aritmetica/09_shr_basico.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 8, "09_shr_basico.vmx desplaza a derecha EAX = 8");

    // 11. logica/01_and_basico.vmx: mov eax, 0b1100; and eax, 0b1010
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/logica/01_and_basico.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 8, "01_and_basico.vmx realiza AND dejando EAX = 8 (0b1000)");

    // 12. logica/02_or_basico.vmx: mov eax, 0b1100; or eax, 0b0011
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/logica/02_or_basico.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 15, "02_or_basico.vmx realiza OR dejando EAX = 15 (0b1111)");

    // 13. logica/03_xor_basico.vmx: mov eax, 0b1111; xor eax, 0b1010
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/logica/03_xor_basico.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 5, "03_xor_basico.vmx realiza XOR dejando EAX = 5 (0b0101)");

    // 14. logica/04_not_basico.vmx: mov eax, 0; not eax
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/logica/04_not_basico.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, -1, "04_not_basico.vmx invierte a EAX = -1");

    // 15. bytes/03_ldl_ldh_combinado.vmx: ldl eax, 0x3456; ldh eax, 0x1200
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/bytes/03_ldl_ldh_combinado.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 0x12003456, "03_ldl_ldh_combinado.vmx compone EAX = 0x12003456");

    // 16. saltos/01_jmp_incondicional.vmx: jmp fin; mov eax, 99; fin: stop
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/saltos/01_jmp_incondicional.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 0, "01_jmp_incondicional.vmx saltea la asignacion intermedia");

    // 17. saltos/02_jz_salta.vmx: sub eax, 10; jz exito; mov ebx, 99; exito: stop
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/saltos/02_jz_salta.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EBX, 0, "02_jz_salta.vmx ejecuta el salto condicional en cero");

    // 18. saltos/03_jnz_no_salta.vmx: jnz no_debe_saltar; mov ebx, 42; fin: stop
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/saltos/03_jnz_no_salta.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EBX, 42, "03_jnz_no_salta.vmx no salta y asigna EBX = 42");

    // 19. saltos/06_jc_salta.vmx: sub 3, 5 genera borrow (C=1) y JC salta
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/saltos/06_jc_salta.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EBX, 1, "06_jc_salta.vmx salta con carry/borrow dejando EBX = 1");

    // 20. saltos/07_jv_salta.vmx: suma maxima genera overflow (V=1) y JV salta
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/saltos/07_jv_salta.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, ECX, 1, "07_jv_salta.vmx salta con overflow dejando ECX = 1");

    // 21. saltos/08_jnp_salta.vmx: 10 - 10 = 0 (Z=1), JNP salta
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/saltos/08_jnp_salta.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EBX, 1, "08_jnp_salta.vmx salta con resultado cero dejando EBX = 1");

    // 22. saltos/09_jnn_salta.vmx: 10 + 5 = 15 (N=0), JNN salta
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/saltos/09_jnn_salta.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EBX, 1, "09_jnn_salta.vmx salta con resultado no negativo dejando EBX = 1");

    // 23. memoria/01_mem_offset_reg.vmx: mov [edx+8], 999
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/memoria/01_mem_offset_reg.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 999, "01_mem_offset_reg.vmx direccionamiento [reg+offset] = 999");

    // 24. memoria/02_mem_offset_neg.vmx: mov [edx-4], 777
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/memoria/02_mem_offset_neg.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 777, "02_mem_offset_neg.vmx direccionamiento [reg-offset] = 777");

    // 25. memoria/03_mem_swap.vmx: swap [0], [4]
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/memoria/03_mem_swap.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 222, "03_mem_swap.vmx intercambia dejando [0] -> EAX = 222");
    assert_registro(&vmx, EBX, 111, "03_mem_swap.vmx intercambia dejando [4] -> EBX = 111");

    // --- ALGORITMOS COMPLEJOS MULTILINEA ---

    // 26. complejos/01_suma_acumulativa.vmx: bucle 1..10 = 55
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/complejos/01_suma_acumulativa.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 55, "01_suma_acumulativa.vmx bucle suma 1..10 = 55");

    // 27. complejos/02_factorial.vmx: calculo de 5! = 120
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/complejos/02_factorial.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 120, "02_factorial.vmx calculo 5! = 120");

    // 28. complejos/03_maximo_vector.vmx: max(15, 82, 33, 64) = 82
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/complejos/03_maximo_vector.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 82, "03_maximo_vector.vmx encuentra maximo = 82");

    // 29. complejos/04_mcd_euclides.vmx: MCD(48, 18) = 6
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/complejos/04_mcd_euclides.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 6, "04_mcd_euclides.vmx calcula MCD(48, 18) = 6");

    // 30. complejos/05_fibonacci.vmx: 7mo termino de Fibonacci = 13
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/complejos/05_fibonacci.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 13, "05_fibonacci.vmx calcula F(7) = 13");

    // 31. complejos/06_invertir_vector.vmx: [10, 20, 30, 40] -> [40, 30, 20, 10]
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/complejos/06_invertir_vector.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 40, "06_invertir_vector.vmx pos 0 = 40");
    assert_registro(&vmx, EBX, 30, "06_invertir_vector.vmx pos 1 = 30");
    assert_registro(&vmx, ECX, 20, "06_invertir_vector.vmx pos 2 = 20");
    assert_registro(&vmx, EDX, 10, "06_invertir_vector.vmx pos 3 = 10");

    // --- ALGORITMOS AVANZADOS CON VECTORES Y MATRICES ---

    // 32. avanzados/01_ordenamiento_burbuja.vmx: [45, 12, 89, 3, 27, 50] -> [3, 12, 27, 45, 50, 89]
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/avanzados/01_ordenamiento_burbuja.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 3, "01_ordenamiento_burbuja.vmx elemento 0 = 3");
    assert_registro(&vmx, EBX, 12, "01_ordenamiento_burbuja.vmx elemento 1 = 12");
    assert_registro(&vmx, ECX, 27, "01_ordenamiento_burbuja.vmx elemento 2 = 27");
    assert_registro(&vmx, EDX, 45, "01_ordenamiento_burbuja.vmx elemento 3 = 45");
    assert_registro(&vmx, EEX, 50, "01_ordenamiento_burbuja.vmx elemento 4 = 50");
    assert_registro(&vmx, EFX, 89, "01_ordenamiento_burbuja.vmx elemento 5 = 89");

    // 33. avanzados/02_multiplicacion_matrices.vmx: C = A x B = [[19, 22], [43, 50]]
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/avanzados/02_multiplicacion_matrices.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 19, "02_multiplicacion_matrices.vmx C[0][0] = 19");
    assert_registro(&vmx, EBX, 22, "02_multiplicacion_matrices.vmx C[0][1] = 22");
    assert_registro(&vmx, ECX, 43, "02_multiplicacion_matrices.vmx C[1][0] = 43");
    assert_registro(&vmx, EDX, 50, "02_multiplicacion_matrices.vmx C[1][1] = 50");

    // 34. avanzados/03_llenado_memoria_maximo.vmx: 3500 enteros (14000 bytes)
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/avanzados/03_llenado_memoria_maximo.vmx", ruta);
    cargar_programa(&vmx, ruta);
    ejecutar_vmx(&vmx);
    assert_registro(&vmx, EAX, 0, "03_llenado_memoria_maximo.vmx primera celda = 0");
    assert_registro(&vmx, EBX, 3499, "03_llenado_memoria_maximo.vmx ultima celda = 3499");

    // --- CASOS CONFLICTIVOS Y DE EXCEPCION ---
    set_abortar_handler(mock_abortar);

    // 35. conflictivos/01_fallo_segmento_exceso.vmx
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/conflictivos/01_fallo_segmento_exceso.vmx", ruta);
    cargar_programa(&vmx, ruta);
    s_abort_llamado = 0;
    s_ultimo_mensaje_abort[0] = '\0';
    if (setjmp(s_jmp_abort) == 0) {
        ejecutar_vmx(&vmx);
    }
    ASSERT(s_abort_llamado == 1, "01_fallo_segmento_exceso.vmx dispara llamada a abortar");
    ASSERT(strstr(s_ultimo_mensaje_abort, "Fallo de segmento") != NULL, "Mensaje reporta 'Fallo de segmento'");

    // 36. conflictivos/02_division_por_cero.vmx
    inicializar_vmx(&vmx, 0, 0);
    resolver_ruta_vmx("vmx/conflictivos/02_division_por_cero.vmx", ruta);
    cargar_programa(&vmx, ruta);
    s_abort_llamado = 0;
    s_ultimo_mensaje_abort[0] = '\0';
    if (setjmp(s_jmp_abort) == 0) {
        ejecutar_vmx(&vmx);
    }
    ASSERT(s_abort_llamado == 1, "02_division_por_cero.vmx dispara llamada a abortar");
    ASSERT(strstr(s_ultimo_mensaje_abort, "Division por cero") != NULL, "Mensaje reporta 'Division por cero'");

    // Restaurar el handler original
    set_abortar_handler(NULL);
}
