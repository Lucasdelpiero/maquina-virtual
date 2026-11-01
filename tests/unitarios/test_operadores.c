#include "test_helpers.h"
#include "../../src/operadores.h"
#include "../../src/operandos.h"
#include "../../src/inicializador.h"

// Helper para configurar OP1
static void set_op1(Vmx *vmx, int tipo, int32_t dato) {
    vmx->registros[OP1] = (tipo << 24) | (dato & 0x00FFFFFF);
}

// Helper para configurar OP2
static void set_op2(Vmx *vmx, int tipo, int32_t dato) {
    vmx->registros[OP2] = (tipo << 24) | (dato & 0x00FFFFFF);
}

void test_operadores_suite(void) {
    printf("\n--- TESTS: operadores.c ---\n");

    Vmx vmx;
    inicializar_vmx(&vmx, 16, 0, 0);
    inicializar_operadores();

    // Test 1: inicializar_operadores registra punteros no nulos
    TEST_ASSERT_NOT_EQUAL((long long)tabla_operaciones[OP_MOV], 0, "inicializar_operadores: OP_MOV registrado");
    TEST_ASSERT_NOT_EQUAL((long long)tabla_operaciones[OP_ADD], 0, "inicializar_operadores: OP_ADD registrado");
    TEST_ASSERT_NOT_EQUAL((long long)tabla_operaciones[OP_STOP], 0, "inicializar_operadores: OP_STOP registrado");
    TEST_ASSERT_NOT_EQUAL((long long)tabla_operaciones[OP_PUSH], 0, "inicializar_operadores: OP_PUSH registrado");
    TEST_ASSERT_NOT_EQUAL((long long)tabla_operaciones[OP_LDL], 0, "inicializar_operadores: OP_LDL registrado");
    TEST_ASSERT_NOT_EQUAL((long long)tabla_operaciones[OP_LDH], 0, "inicializar_operadores: OP_LDH registrado");

    // Test 2: actualizar_cc
    actualizar_cc(&vmx, -5, 0, 0); // N=1
    TEST_ASSERT_EQUAL((uint32_t)vmx.registros[CC], (uint32_t)0x80000000, "actualizar_cc: N=1 cuando res es negativo");

    actualizar_cc(&vmx, 0, 0, 0); // Z=1
    TEST_ASSERT_EQUAL((uint32_t)vmx.registros[CC], (uint32_t)0x40000000, "actualizar_cc: Z=1 cuando res es 0");

    actualizar_cc(&vmx, 10, 1, 1); // C=1, V=1
    TEST_ASSERT_EQUAL((uint32_t)vmx.registros[CC], (uint32_t)0x30000000, "actualizar_cc: C=1 y V=1 empaquetados correctamente");

    // Test 3: op_mov
    set_op1(&vmx, TIPO_REGISTRO, EAX);
    set_op2(&vmx, TIPO_INMEDIATO, 1234);
    op_mov(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], 1234, "op_mov: asigna valor inmediato a registro");
    TEST_ASSERT_EQUAL((uint32_t)vmx.registros[CC], (uint32_t)0, "op_mov: CC en 0 para positivo");

    set_op2(&vmx, TIPO_INMEDIATO, (uint16_t)-1);
    op_mov(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], -1, "op_mov: asigna valor negativo");
    TEST_ASSERT_EQUAL((uint32_t)vmx.registros[CC], (uint32_t)0x80000000, "op_mov: CC activa N=1");

    // Test 4: op_add (casos normales, suma cero y overflow)
    vmx.registros[EAX] = 2;
    set_op1(&vmx, TIPO_REGISTRO, EAX);
    set_op2(&vmx, TIPO_INMEDIATO, 2);
    op_add(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], 4, "op_add: 2 + 2 = 4");
    TEST_ASSERT_EQUAL((uint32_t)vmx.registros[CC], (uint32_t)0, "op_add: resultado positivo sin carry ni overflow deja CC=0");

    vmx.registros[EAX] = -15;
    set_op2(&vmx, TIPO_INMEDIATO, 15);
    op_add(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], 0, "op_add: -15 + 15 = 0");
    TEST_ASSERT_EQUAL((uint32_t)vmx.registros[CC] & 0x40000000, (uint32_t)0x40000000, "op_add: resultado 0 activa Z=1");

    // Overflow: INT32_MAX + INT32_MAX
    vmx.registros[EAX] = 2147483647;
    vmx.registros[EBX] = 2147483647;
    set_op2(&vmx, TIPO_REGISTRO, EBX);
    op_add(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], -2, "op_add: INT32_MAX + INT32_MAX = -2");
    TEST_ASSERT_EQUAL((uint32_t)vmx.registros[CC], (uint32_t)0x90000000, "op_add: flags N=1 y V=1 en overflow");

    // Test 5: op_sub (casos normales, resultado cero, borrow y negativos)
    vmx.registros[EAX] = 10;
    set_op1(&vmx, TIPO_REGISTRO, EAX);
    set_op2(&vmx, TIPO_INMEDIATO, 6);
    op_sub(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], 4, "op_sub: 10 - 6 = 4");

    vmx.registros[EAX] = 25;
    set_op2(&vmx, TIPO_INMEDIATO, 25);
    op_sub(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], 0, "op_sub: 25 - 25 = 0");
    TEST_ASSERT_EQUAL((uint32_t)vmx.registros[CC] & 0x40000000, (uint32_t)0x40000000, "op_sub: resultado 0 activa flag Z=1");

    vmx.registros[EAX] = -3;
    vmx.registros[EBX] = 5;
    set_op2(&vmx, TIPO_REGISTRO, EBX);
    op_sub(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], -8, "op_sub: -3 - 5 = -8");
    TEST_ASSERT_EQUAL((uint32_t)vmx.registros[CC], (uint32_t)0xA0000000, "op_sub: flags N=1 y C=1");

    // Test 6: op_mul (casos normales, cero, negativos y overflow)
    vmx.registros[EAX] = 2;
    set_op1(&vmx, TIPO_REGISTRO, EAX);
    set_op2(&vmx, TIPO_INMEDIATO, 2);
    op_mul(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], 4, "op_mul: 2 * 2 = 4");

    vmx.registros[EAX] = 42;
    set_op2(&vmx, TIPO_INMEDIATO, 0);
    op_mul(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], 0, "op_mul: 42 * 0 = 0");
    TEST_ASSERT_EQUAL((uint32_t)vmx.registros[CC], (uint32_t)0x40000000, "op_mul: multiplica por 0 activa flag Z=1");

    vmx.registros[EAX] = -5;
    set_op2(&vmx, TIPO_INMEDIATO, 4);
    op_mul(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], -20, "op_mul: -5 * 4 = -20");
    TEST_ASSERT_EQUAL((uint32_t)vmx.registros[CC] & 0x80000000, (uint32_t)0x80000000, "op_mul: resultado negativo activa N=1");

    vmx.registros[EAX] = 1073741827; // 0x40000003
    vmx.registros[EBX] = 4;
    set_op2(&vmx, TIPO_REGISTRO, EBX);
    op_mul(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], 12, "op_mul: 1073741827 * 4 = 12 (truncado a 32 bits)");
    TEST_ASSERT_EQUAL((uint32_t)vmx.registros[CC], (uint32_t)0x30000000, "op_mul: flags C=1 y V=1");

    // Test 7: op_div (division exacta, con resto en AC, con signo y division por cero)
    vmx.registros[EAX] = 8;
    set_op1(&vmx, TIPO_REGISTRO, EAX);
    set_op2(&vmx, TIPO_INMEDIATO, 2);
    op_div(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], 4, "op_div: 8 / 2 = 4");
    TEST_ASSERT_EQUAL(vmx.registros[AC], 0, "op_div: resto 8 % 2 = 0 en AC");

    vmx.registros[EAX] = 17;
    vmx.registros[EBX] = 5;
    set_op2(&vmx, TIPO_REGISTRO, EBX);
    op_div(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], 3, "op_div: 17 / 5 = 3");
    TEST_ASSERT_EQUAL(vmx.registros[AC], 2, "op_div: resto 17 % 5 = 2 en AC");

    // Division con negativos
    vmx.registros[EAX] = -17;
    vmx.registros[EBX] = 5;
    op_div(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], -3, "op_div: -17 / 5 = -3");
    TEST_ASSERT_EQUAL(vmx.registros[AC], -2, "op_div: resto -17 % 5 = -2 en AC");

    vmx.registros[EAX] = 17;
    vmx.registros[EBX] = -5;
    op_div(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], -3, "op_div: 17 / -5 = -3");
    TEST_ASSERT_EQUAL(vmx.registros[AC], 2, "op_div: resto 17 % -5 = 2 en AC");

    // Division por cero aborta
    vmx.registros[EBX] = 0;
    TEST_ASSERT_ABORTA(op_div(&vmx), "op_div: division por cero aborta inmediatamente");

    // Test 8: op_cmp (preserva OP1 y actualiza CC)
    vmx.registros[EAX] = 50;
    vmx.registros[EBX] = 50;
    set_op1(&vmx, TIPO_REGISTRO, EAX);
    set_op2(&vmx, TIPO_REGISTRO, EBX);
    op_cmp(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], 50, "op_cmp: valor de operando A preservado");
    TEST_ASSERT_EQUAL((uint32_t)vmx.registros[CC], (uint32_t)0x40000000, "op_cmp: iguales activa Z=1");

    vmx.registros[EBX] = 60;
    op_cmp(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], 50, "op_cmp: A menor que B");
    TEST_ASSERT_EQUAL((uint32_t)vmx.registros[CC] & 0x80000000, (uint32_t)0x80000000, "op_cmp: A menor que B activa N=1");

    // Test 9: Logica (AND, OR, XOR, NOT)
    vmx.registros[EAX] = 0xF0;
    vmx.registros[EBX] = 0x0F;
    set_op1(&vmx, TIPO_REGISTRO, EAX);
    set_op2(&vmx, TIPO_REGISTRO, EBX);
    op_and(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], 0, "op_and: 0xF0 & 0x0F = 0");
    TEST_ASSERT_EQUAL((uint32_t)vmx.registros[CC], (uint32_t)0x40000000, "op_and: Z=1");

    op_or(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], 0x0F, "op_or: 0 | 0x0F = 0x0F");

    vmx.registros[EBX] = 0x0F;
    op_xor(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], 0, "op_xor: 0x0F ^ 0x0F = 0");

    set_op1(&vmx, TIPO_REGISTRO, EAX);
    op_not(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], -1, "op_not: ~0 = -1 (0xFFFFFFFF)");
    TEST_ASSERT_EQUAL((uint32_t)vmx.registros[CC], (uint32_t)0x80000000, "op_not: N=1");

    // Test 10: Desplazamientos (SHL, SHR, SAR)
    vmx.registros[EAX] = 1;
    set_op1(&vmx, TIPO_REGISTRO, EAX);
    set_op2(&vmx, TIPO_INMEDIATO, 3);
    op_shl(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], 8, "op_shl: 1 << 3 = 8");

    // Desplazamiento 0
    vmx.registros[EAX] = 42;
    set_op2(&vmx, TIPO_INMEDIATO, 0);
    op_shl(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], 42, "op_shl: desplazamiento 0 mantiene valor");

    vmx.registros[EAX] = 16;
    set_op2(&vmx, TIPO_INMEDIATO, 2);
    op_shr(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], 4, "op_shr: 16 >> 2 = 4");

    // SHR de valor con bit 31 en 1 (desplazamiento logico sin propagar signo)
    vmx.registros[EAX] = (int32_t)0x80000000;
    set_op2(&vmx, TIPO_INMEDIATO, 1);
    op_shr(&vmx);
    TEST_ASSERT_EQUAL((uint32_t)vmx.registros[EAX], (uint32_t)0x40000000, "op_shr: 0x80000000 >> 1 logico = 0x40000000");

    // SAR de valor negativo (desplazamiento aritmetico propagando signo)
    vmx.registros[EAX] = -16; // 0xFFFFFFF0
    set_op2(&vmx, TIPO_INMEDIATO, 2);
    op_sar(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], -4, "op_sar: -16 >> 2 aritmetico = -4");

    // Test 11: SWAP
    vmx.registros[EAX] = 100;
    vmx.registros[EBX] = 200;
    set_op1(&vmx, TIPO_REGISTRO, EAX);
    set_op2(&vmx, TIPO_REGISTRO, EBX);
    op_swap(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], 200, "op_swap: EAX ahora vale 200");
    TEST_ASSERT_EQUAL(vmx.registros[EBX], 100, "op_swap: EBX ahora vale 100");

    // Test 12: LDL y LDH (en registros y memoria)
    // LDL en registro EAX
    vmx.registros[EAX] = 0x12345678;
    set_op1(&vmx, TIPO_REGISTRO, EAX);
    set_op2(&vmx, TIPO_INMEDIATO, 0xABCD);
    vmx.registros[CC] = 0x12345678; // Verificamos que LDL no toca CC
    op_ldl(&vmx);
    TEST_ASSERT_EQUAL((uint32_t)vmx.registros[EAX], (uint32_t)0x1234ABCD, "op_ldl: carga 16 bits bajos preservando altos");
    TEST_ASSERT_EQUAL(vmx.registros[CC], 0x12345678, "op_ldl: no modifica registro CC");

    // LDH en registro EAX
    set_op2(&vmx, TIPO_INMEDIATO, 0x9988);
    op_ldh(&vmx);
    TEST_ASSERT_EQUAL((uint32_t)vmx.registros[EAX], (uint32_t)0x9988ABCD, "op_ldh: carga 16 bits altos preservando bajos");
    TEST_ASSERT_EQUAL(vmx.registros[CC], 0x12345678, "op_ldh: no modifica registro CC");

    // LDL y LDH sobre memoria
    vmx.memoria.tabla_segmentos[0] = (0 << 16) | 1000;
    vmx.registros[DS] = 0x00000000;
    escribir_memoria(&vmx.memoria, 50, 4, 0x11223344);
    int32_t dato_mem_50 = (50 << 8) | DS;
    set_op1(&vmx, TIPO_MEMORIA, dato_mem_50);
    set_op2(&vmx, TIPO_INMEDIATO, 0x7777);
    op_ldl(&vmx);
    TEST_ASSERT_EQUAL((uint32_t)leer_memoria(&vmx.memoria, 50, 4), (uint32_t)0x11227777, "op_ldl en memoria: modifica parte baja");

    set_op2(&vmx, TIPO_INMEDIATO, 0x8888);
    op_ldh(&vmx);
    TEST_ASSERT_EQUAL((uint32_t)leer_memoria(&vmx.memoria, 50, 4), (uint32_t)0x88887777, "op_ldh en memoria: modifica parte alta");

    // Test 13: Saltos incondicionales y condicionales (probando ambos caminos: cuando salta y cuando no salta)
    vmx.registros[CS] = 0x00000000;
    vmx.memoria.tabla_segmentos[0] = (0 << 16) | 1000;

    // STOP
    vmx.registros[IP] = 0;
    op_stop(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[IP], -1, "op_stop: IP queda en -1");

    // JMP
    set_op1(&vmx, TIPO_INMEDIATO, 120);
    op_jmp(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[IP], 120, "op_jmp: IP salta incondicionalmente a 120");

    // JZ / JNZ
    vmx.registros[CC] = 0x40000000; // Z=1
    set_op1(&vmx, TIPO_INMEDIATO, 200);
    op_jz(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[IP], 200, "op_jz: salta cuando Z=1");

    vmx.registros[CC] = 0x00000000; // Z=0
    vmx.registros[IP] = 10;
    op_jz(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[IP], 10, "op_jz: no salta cuando Z=0");

    vmx.registros[IP] = 10;
    op_jnz(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[IP], 200, "op_jnz: salta cuando Z=0");

    vmx.registros[CC] = 0x40000000; // Z=1
    vmx.registros[IP] = 10;
    op_jnz(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[IP], 10, "op_jnz: no salta cuando Z=1");

    // JN / JNN
    vmx.registros[CC] = 0x80000000; // N=1
    set_op1(&vmx, TIPO_INMEDIATO, 300);
    op_jn(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[IP], 300, "op_jn: salta cuando N=1");

    vmx.registros[CC] = 0x00000000; // N=0
    vmx.registros[IP] = 10;
    op_jn(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[IP], 10, "op_jn: no salta cuando N=0");

    vmx.registros[IP] = 10;
    op_jnn(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[IP], 300, "op_jnn: salta cuando N=0");

    vmx.registros[CC] = 0x80000000; // N=1
    vmx.registros[IP] = 10;
    op_jnn(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[IP], 10, "op_jnn: no salta cuando N=1");

    // JP / JNP
    vmx.registros[CC] = 0; // N=0, Z=0 -> positivo estricto
    set_op1(&vmx, TIPO_INMEDIATO, 400);
    op_jp(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[IP], 400, "op_jp: salta cuando positivo (N=0, Z=0)");

    vmx.registros[CC] = 0x40000000; // Z=1 -> cero no es positivo estricto
    vmx.registros[IP] = 10;
    op_jp(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[IP], 10, "op_jp: no salta cuando cero (Z=1)");

    vmx.registros[CC] = 0x80000000; // N=1 -> negativo no es positivo
    vmx.registros[IP] = 10;
    op_jp(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[IP], 10, "op_jp: no salta cuando negativo (N=1)");

    // JNP: salta si N=1 o Z=1 (no positivo)
    vmx.registros[IP] = 10;
    op_jnp(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[IP], 400, "op_jnp: salta cuando negativo (N=1)");

    vmx.registros[CC] = 0x40000000; // Z=1
    vmx.registros[IP] = 10;
    op_jnp(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[IP], 400, "op_jnp: salta cuando cero (Z=1)");

    vmx.registros[CC] = 0x00000000; // N=0, Z=0 -> positivo
    vmx.registros[IP] = 10;
    op_jnp(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[IP], 10, "op_jnp: no salta cuando positivo (N=0, Z=0)");

    // JC
    vmx.registros[CC] = 0x20000000; // C=1
    set_op1(&vmx, TIPO_INMEDIATO, 500);
    op_jc(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[IP], 500, "op_jc: salta cuando C=1");

    vmx.registros[CC] = 0x00000000; // C=0
    vmx.registros[IP] = 10;
    op_jc(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[IP], 10, "op_jc: no salta cuando C=0");

    // JV
    vmx.registros[CC] = 0x10000000; // V=1
    set_op1(&vmx, TIPO_INMEDIATO, 600);
    op_jv(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[IP], 600, "op_jv: salta cuando V=1");

    vmx.registros[CC] = 0x00000000; // V=0
    vmx.registros[IP] = 10;
    op_jv(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[IP], 10, "op_jv: no salta cuando V=0");

    // Test 14: Manejo de Pila completo y sacar_pila
    // Configuramos Stack Segment en entrada 1: base 500, tamano 32 bytes
    vmx.memoria.tabla_segmentos[1] = (500 << 16) | 32;
    vmx.registros[SS] = 0x00010000;
    vmx.registros[SP] = 0x00010020; // offset 32 (tope vacio)

    // sacar_pila con pila vacia aborta (UNDERFLOW)
    TEST_ASSERT_ABORTA(sacar_pila(&vmx), "sacar_pila: extraccion en pila vacia dispara STACK UNDERFLOW");

    // PUSH inmediato
    set_op1(&vmx, TIPO_INMEDIATO, 42);
    op_push(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[SP], 0x0001001C, "op_push inmediato: SP decrementado a 28");

    // sacar_pila devuelve el 42 y SP vuelve a 32
    int32_t sacado = sacar_pila(&vmx);
    TEST_ASSERT_EQUAL(sacado, 42, "sacar_pila: devuelve 42 almacenado en pila");
    TEST_ASSERT_EQUAL(vmx.registros[SP], 0x00010020, "sacar_pila: incrementa SP en 4");

    // Caso especificacion PUSH AX con extension de signo:
    // EAX = 0x1234ABCD -> AX = 0xABCD (-21555) -> extendido a 32 bits = 0xFFFFABCD
    vmx.registros[EAX] = 0x1234ABCD;
    set_op1(&vmx, TIPO_REGISTRO, (3 << 5) | EAX); // sector 3 = AX
    op_push(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[SP], 0x0001001C, "op_push AX: decrementa SP");
    // Verificamos bytes big-endian en memoria fisica de la pila (base 500 + 28 = 528)
    TEST_ASSERT_EQUAL(vmx.memoria.mem_principal[528], 0xFF, "op_push AX: byte 0 en tope es 0xFF");
    TEST_ASSERT_EQUAL(vmx.memoria.mem_principal[529], 0xFF, "op_push AX: byte 1 es 0xFF");
    TEST_ASSERT_EQUAL(vmx.memoria.mem_principal[530], 0xAB, "op_push AX: byte 2 es 0xAB");
    TEST_ASSERT_EQUAL(vmx.memoria.mem_principal[531], 0xCD, "op_push AX: byte 3 es 0xCD");

    // Caso especificacion POP w[ES+4] con truncamiento:
    // Configuramos ES en entrada 2: base 600, tamano 50
    vmx.memoria.tabla_segmentos[2] = (600 << 16) | 50;
    vmx.registros[ES] = 0x00020000;
    int32_t dato_mem_es4 = (2 << 5) | (4 << 8) | ES; // w[ES+4]
    set_op1(&vmx, TIPO_MEMORIA, dato_mem_es4);
    op_pop(&vmx);
    // En byte 4 del Extra Segment (600 + 4 = 604) debe quedar 0xAB y en byte 5 0xCD
    TEST_ASSERT_EQUAL(vmx.memoria.mem_principal[604], 0xAB, "op_pop w[ES+4]: byte 4 en ES es 0xAB");
    TEST_ASSERT_EQUAL(vmx.memoria.mem_principal[605], 0xCD, "op_pop w[ES+4]: byte 5 en ES es 0xCD");
    TEST_ASSERT_EQUAL(vmx.registros[SP], 0x00010020, "op_pop: SP restablecido");

    // POP asignando a sector AL
    vmx.registros[EAX] = 0x11223344;
    set_op1(&vmx, TIPO_INMEDIATO, 0x00000077);
    op_push(&vmx);
    set_op1(&vmx, TIPO_REGISTRO, (1 << 5) | EAX); // AL
    op_pop(&vmx);
    TEST_ASSERT_EQUAL((uint32_t)vmx.registros[EAX], (uint32_t)0x11223377, "op_pop AL: modifica solo AL truncando a 8 bits");

    // Desborde forzado (STACK OVERFLOW): Pila de 32 bytes admite 8 PUSH de 4 bytes
    vmx.registros[SP] = 0x00010004; // Espacio para exactamente 1 PUSH mas
    set_op1(&vmx, TIPO_INMEDIATO, 1);
    op_push(&vmx); // SP = 0x00010000 (tope exacto de SS)
    TEST_ASSERT_EQUAL(vmx.registros[SP], 0x00010000, "op_push: pila llena en limite exacto");
    TEST_ASSERT_ABORTA(op_push(&vmx), "op_push: siguiente PUSH dispara STACK OVERFLOW");

    // CALL y RET
    vmx.registros[SP] = 0x00010010;
    vmx.registros[IP] = 0x00000050; // Siguiente instruccion
    set_op1(&vmx, TIPO_INMEDIATO, 180); // Destino de subrutina
    op_call(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[IP], 0x000000B4, "op_call: salta a destino 180 (0xB4)");

    op_ret(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[IP], 0x00000050, "op_ret: retorna a direccion original guardada en pila");

    // Test 15: RND
    set_op1(&vmx, TIPO_REGISTRO, EAX);
    set_op2(&vmx, TIPO_INMEDIATO, 10);
    op_rnd(&vmx);
    TEST_ASSERT(vmx.registros[EAX] >= 0 && vmx.registros[EAX] < 10, "op_rnd: valor generado en rango [0, 9]");

    set_op2(&vmx, TIPO_INMEDIATO, 0);
    op_rnd(&vmx);
    TEST_ASSERT_EQUAL(vmx.registros[EAX], 0, "op_rnd: limite <= 0 genera 0");

    set_op2(&vmx, TIPO_INMEDIATO, (uint16_t)-5);
    TEST_ASSERT_ABORTA(op_rnd(&vmx), "op_rnd: limite negativo aborta");

    free(vmx.memoria.mem_principal);
}
