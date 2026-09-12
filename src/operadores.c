#include "operadores.h"

// Actualiza los bits N, Z, C, V del registro CC
void actualizar_cc(Vmx *vmx, int32_t resultado, int carry, int overflow) {
    uint32_t n;
    uint32_t z;
    uint32_t c;
    uint32_t v;

    // Bit 31 (N): 1 si el resultado de 32 bits con signo es negativo
    n = ((uint32_t)resultado >> 31) & 1;

    // Bit 30 (Z): 1 si el resultado de 32 bits es cero
    z = (resultado == 0) ? 1 : 0;

    // Bit 29 (C): 1 si se produjo acarreo sin signo
    c = carry ? 1 : 0;

    // Bit 28 (V): 1 si se produjo desbordamiento con signo
    v = overflow ? 1 : 0;

    // Empaqueta los 4 flags en los 4 bits mas significativos; bits 0..27 quedan en cero
    vmx->registros[CC] = (int32_t)((n << 31) | (z << 30) | (c << 29) | (v << 28));
}
