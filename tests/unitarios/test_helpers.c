#include "test_helpers.h"

int g_tests_ejecutados = 0;
int g_tests_aprobados = 0;
int g_tests_fallados = 0;

jmp_buf g_salto_abortar;
int g_aborto_disparado = 0;
char g_ultimo_mensaje_aborto[512] = "";

void capturador_abortos(char mensaje[]) {
    g_aborto_disparado = 1;
    if (mensaje != NULL) {
        strncpy(g_ultimo_mensaje_aborto, mensaje, sizeof(g_ultimo_mensaje_aborto) - 1);
        g_ultimo_mensaje_aborto[sizeof(g_ultimo_mensaje_aborto) - 1] = '\0';
    } else {
        g_ultimo_mensaje_aborto[0] = '\0';
    }
    longjmp(g_salto_abortar, 1);
}
