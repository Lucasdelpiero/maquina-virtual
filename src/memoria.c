#include "memoria.h"

// Stubs temporales: Lucas implementara la logica real de la Etapa 3
uint16_t traducir_direccion(Vmx *vmx, int32_t dir_logica, uint16_t cant_bytes) {
    (void)vmx;
    (void)dir_logica;
    (void)cant_bytes;
    return 0;
}

int32_t leer_memoria(Vmx *vmx, uint16_t dir_fisica, uint8_t cant_bytes) {
    (void)vmx;
    (void)dir_fisica;
    (void)cant_bytes;
    return 0;
}

void escribir_memoria(Vmx *vmx, uint16_t dir_fisica, uint8_t cant_bytes, int32_t valor) {
    (void)vmx;
    (void)dir_fisica;
    (void)cant_bytes;
    (void)valor;
}
