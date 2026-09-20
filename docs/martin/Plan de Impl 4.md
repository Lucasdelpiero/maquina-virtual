# Apuntes de diseño — Máquina Virtual (vmx)

_Fundamentos de la Arquitectura de Computadoras — UNMDP — MV1 2026_

## 1. Contexto del proyecto

El trabajo consiste en implementar, en C, la **máquina virtual (vmx)** que ejecuta programas ya traducidos a lenguaje máquina (`.vmx`). La traducción de Assembler (`.asm`) a binario la hace el **traductor `vmt`, provisto por la cátedra** — no es parte de lo que construimos.

Por línea de comando, a la VM se le pasa el **nombre del archivo `.vmx`** a ejecutar (no el binario directamente):

```
vmx filename.vmx [-d]
```

El flag `-d` activa el disassembler (ver sección 6).

## 2. Equipo y roles

Equipo de 3 personas. **Los 3 son arquitectos de la solución** — diseño, revisión de código y implementación general son responsabilidad compartida.

- **Renzo, Lucas y Martin**: diseño de solución, reviewer general, implementaciones generales.
- **Martin**: además, arma los tests con ayuda de herramientas de IA (por su mayor manejo de este tipo de herramientas).

Regla de trabajo en equipo: en lo posible, que no más de una persona edite el mismo archivo al mismo tiempo (coordinar por ramas en el repositorio) para evitar conflictos al mergear.

## 3. Principios de diseño (cerrados)

- Código en **C clásico/básico**, evitando atajos o construcciones poco convencionales.
- Nombres propios del código (funciones, variables, comentarios) **en español**. Los mnemónicos y nombres de registros se mantienen tal cual los define la cátedra (MOV, EAX, CS, etc.), no se traducen.
- Diseño basado en **TDA**: structs + punteros para representar la VM, los registros, la memoria, la tabla de segmentos, etc.
- Funciones cortas y archivos con pocas líneas — evitar módulos monolíticos.
- **Sin variables globales (`extern`)**: el struct de la VM se pasa siempre explícitamente por puntero a cada función que lo necesite.
- El proyecto se pensará expansible, ya que se extenderá en etapas futuras.
- No usar `switch` como excepción a lo "clásico": si el patrón de datos lo permite (ver sección 4.2), se prioriza una estructura de datos con índice significativo (array + enum) antes que una cadena de comparaciones.
- Tipos de tamaño fijo (`int32_t`, `uint32_t`, `uint8_t`, etc.) requieren `#include <stdint.h>` en cada archivo que los use.
- **Convención de módulos (TDA) en C**: cada módulo tiene un `.h` con el `typedef struct` y los prototipos de sus funciones públicas, y un `.c` con la implementación. C no tiene funciones anidadas ni sintaxis de método (`objeto.funcion()`) — toda función recibe el puntero al struct que necesita como parámetro explícito.

## 4. Estructuras de datos centrales

### 4.1 Registros

```c
typedef enum {
    IP = 0, OPC = 1, OP1 = 2, OP2 = 3, LAR = 4, MAR = 5, MBR = 6,
    // 7, 8, 9: reservados
    EAX = 10, EBX = 11, ECX = 12, EDX = 13, EEX = 14, EFX = 15,
    AC = 16, CC = 17,
    // 18-25: reservados
    CS = 26, DS = 27
    // 28-31: reservados
} CodigoRegistro;

int32_t registros[32];
```

**Por qué `int32_t` (con signo) y no un array de bits:** los registros almacenan resultados de operaciones aritméticas que pueden dar negativos (restas, inmediatos con signo de -32768 a 32767), y el bit N del registro CC se define justamente en base al signo del resultado. El signo es información real del dominio.

**Por qué enum + array y no un struct con un campo por registro:** el operando de tipo registro llega desde el binario decodificado como un **número en tiempo de ejecución** (0 a 31), no como un nombre fijo en tiempo de compilación. Con un struct (`regs.EBX`) no hay forma de indexar dinámicamente sin un `switch` adicional que traduzca código→campo, duplicando lógica en cada acceso. Con enum + array, `registros[EBX]` es igual de legible al escribir código a mano, y `registros[codigo]` funciona directo cuando el código viene decodificado del binario — cubre los dos casos de uso reales del proyecto sin sacrificar claridad.

### 4.2 Operaciones (reemplazo del switch)

```c
typedef enum {
    OP_SYS = 0x00, OP_JMP = 0x01, OP_JP = 0x02, OP_JN = 0x03, OP_JZ = 0x04,
    OP_JC = 0x05, OP_JV = 0x06, OP_JNP = 0x07, OP_JNN = 0x08, OP_JNZ = 0x09,
    OP_NOT = 0x0A,
    // 0x0B a 0x0E: sin uso
    OP_STOP = 0x0F,
    OP_MOV = 0x10, OP_ADD = 0x11, OP_SUB = 0x12, OP_MUL = 0x13, OP_DIV = 0x14,
    OP_CMP = 0x15, OP_AND = 0x16, OP_OR = 0x17, OP_XOR = 0x18, OP_SWAP = 0x19,
    OP_SHL = 0x1A, OP_SHR = 0x1B, OP_SAR = 0x1C, OP_LDL = 0x1D, OP_LDH = 0x1E,
    OP_RND = 0x1F
} CodigoOperacion;

typedef void (*FuncionOperacion)(VMX *vm);

FuncionOperacion tabla_operaciones[32] = {0}; // huecos quedan en NULL

tabla_operaciones[OP_MOV] = op_mov;
tabla_operaciones[OP_ADD] = op_add;
// ...
```

Al ejecutar: `FuncionOperacion f = tabla_operaciones[vm->registros[OPC]];` — si `f == NULL`, es una instrucción inválida (error a reportar y abortar). Los índices sin uso (0x0B-0x0E) quedan en `NULL` por la inicialización, sin necesidad de manejo especial.

**Detalle específico no cubierto por la especificación:** DIV guarda el resto en AC; RND genera un número aleatorio entre 0 y B guardándolo en A; la familia de saltos condicionales (JP, JN, JZ, JC, JV, JNP, JNN, JNZ) evalúa los bits N/Z/C/V del registro CC.

**Llamadas al sistema (SYS):** EAX indica READ (1) o WRITE (2). EDX contiene la posición de memoria inicial, y ECX la cantidad de celdas (2 bytes menos significativos) y el tamaño de cada una (2 bytes más significativos). Por cada dato accedido se debe mostrar en pantalla un prompt `[XXXX]:` con la dirección física en 4 dígitos hexadecimales.

### 4.3 Memoria principal

```c
uint8_t memoria[16384]; // 16 KiB, fijo según especificación
```

**Por qué `uint8_t` (8 bits, sin signo) y no `int32_t`:** la memoria es un espacio de bytes individuales direccionables uno por uno — la especificación permite que un dato de 4 bytes empiece en cualquier offset, no solo en múltiplos de 4. Con `uint8_t memoria[16384]`, leer/escribir un dato de 4 bytes en la dirección física `X` es simplemente acceder a `memoria[X]` .. `memoria[X+3]`. Sin signo porque el byte representa contenido crudo (instrucción, dato, carácter ASCII), no una cantidad que pueda ser negativa; con signo, un byte como `0xFF` se interpretaría como `-1` y arrastraría signo en operaciones de armado de valores multi-byte (shifts, OR).

### 4.4 Tabla de descriptores de segmentos

```c
uint32_t tabla_segmentos[8];
```

Cada entrada combina base (16 bits altos) y tamaño (16 bits bajos) en un solo `uint32_t`:

```c
uint32_t entrada = tabla_segmentos[1];
uint16_t base   = (entrada >> 16) & 0xFFFF;
uint16_t tamano = entrada & 0xFFFF;
```

**Por qué un solo `uint32_t` por entrada y no separar en dos arrays:** el valor reservado de la especificación es `0xFFFFFFFF`, que representa directamente como un único entero de 32 bits sin tener que poner `0xFFFF` en dos arrays separados. **Sin signo** porque las validaciones de acceso son comparaciones de direcciones (`Base ≤ Dirección Física`, `Límite del segmento ≥ Límite de acceso`), que conceptualmente nunca son negativas; interpretar `0xFFFFFFFF` como `int32_t` lo convierte en `-1`, obligando a cuidado extra en cada comparación y shift (el corrimiento a la derecha de un número negativo no está garantizado por el estándar de C).

**Regla general de tipos:** con signo cuando el valor puede ser negativo por definición del dominio (registros); sin signo cuando el valor es una dirección, un tamaño o contenido binario crudo (memoria, tabla de segmentos).

## 5. Módulos del proyecto

### 5.1 Struct Vmx (central)

Agrupa punteros a lo que expone cada módulo — no los datos en sí:

```c
typedef struct {
    int32_t *registros;
    uint8_t *memoria;
    uint32_t *tabla_segmentos;
} Vmx;
```

Cada módulo (registros, memoria) reserva y define su propia estructura de datos puertas adentro; el struct Vmx solo guarda el puntero. Se pasa por puntero a toda función que lo necesite.

### 5.2 Inicializador

Módulo/archivo aparte, responsable de crear el struct Vmx y asignarle los punteros devueltos por la inicialización de cada módulo (`vmx->registros = registros_inicializar(); vmx->memoria = memoria_inicializar(); ...`).

### 5.3 Carga del programa (lectura de cabecera)

El `.vmx` generado por el traductor empieza con una cabecera de 8 bytes, según la especificación:

|Byte(s)|Campo|Valor|
|---|---|---|
|0-4|Identificador|`"VMX26"`|
|5|Versión|1|
|6-7|Tamaño del código|en bytes|

⚠️ Confirmar con un `.vmx` real generado por el `vmt` de la cátedra que el identificador y la versión coinciden exactamente con estos valores antes de hardcodear la validación — no asumir sin chequear un archivo generado por la herramienta que van a usar en la entrega.

Al cargar el programa: validar identificador y versión (si no coinciden, abortar la carga); cargar el código a partir del byte 8 en la memoria principal, en la zona correspondiente al segmento de código; armar la tabla de segmentos (entrada 0 = código, entrada 1 = datos, resto en -1); inicializar `CS`, `DS`, `IP = CS`.

### 5.4 Ciclo de ejecución (Máquina Virtual)

Ciclo fetch-decode-execute con corte por IP inválido (asignado a `-1` por STOP):

```c
while (ip_valido(vmx)) {
    decodificar(vmx);
    ejecutar(vmx);
}
```

### 5.5 Decodificador de binario

Dado el binario cargado en memoria, obtiene y separa OPC, tipo y valor de OP1/OP2, mediante funciones separadas (`leer_instruccion`, `leer_operandos` — nunca anidadas, C no lo permite). Proceso de lectura:

1. Leer el código de operación (últimos 5 bits del primer byte).
2. Según el código de operación, determinar si la instrucción tiene 2, 1 o 0 operandos.
3. Leer los tipos de operando codificados en el primer byte.
4. Leer los valores de los operandos, que vienen a continuación del primer byte.
5. **Los operandos se codifican en el binario en orden inverso al del Assembler**: primero aparece el operando B, luego el A.
6. Guardar el resultado empaquetado (byte alto = tipo, resto = valor/posición) en OP1 y OP2.

⚠️ El detalle exacto del layout de bits del primer byte conviene verificarlo generando un `.vmx` real con el `vmt` de la cátedra para una instrucción de dos operandos simple y revisando los bytes con un hexdump, en vez de confiar solo en la redacción de la especificación.

### 5.6 Operandos — código final consolidado

Módulo compartido por todos los operadores, para interpretar y guardar un operando ya decodificado (evita que cada operador repita la lógica de "¿es registro, inmediato o memoria?"):

```c
uint8_t getTipo(int32_t operando) { /* extrae el byte alto */ }
int32_t getDato(int32_t operando) { /* extrae el resto */ }

uint16_t resolverDireccionMemoria(Vmx *vmx, int32_t dato) {
    uint8_t codReg = dato & 0x1F;
    int16_t offset = (dato >> 8) & 0xFFFF;
    int32_t base = vmx->registros[codReg];
    return traducirDireccion(vmx, base, offset); // combina y traduce a física
}

int32_t getValor(Vmx *vmx, uint8_t tipo, int32_t dato) {
    if (tipo == TIPO_REGISTRO) return vmx->registros[dato];
    if (tipo == TIPO_INMEDIATO) return dato;
    if (tipo == TIPO_MEMORIA) {
        uint16_t posFisica = resolverDireccionMemoria(vmx, dato);
        return getValorMemoria(vmx, posFisica, 4);
    }
    // error: tipo inválido
}

void guardarRes(Vmx *vmx, uint8_t tipo, int32_t dato, int32_t valor) {
    if (tipo == TIPO_REGISTRO) vmx->registros[dato] = valor;
    else if (tipo == TIPO_MEMORIA) {
        uint16_t posFisica = resolverDireccionMemoria(vmx, dato);
        setValor(vmx, posFisica, 4, valor);
    }
    // destino inmediato: error de operando inválido
}
```

### 5.7 Operadores y tabla de dispatch

Todas las funciones de operación comparten la misma firma (`void (*)(Vmx*)`), sin importar cuántos operandos tenga la instrucción — cada una lee OP1/OP2 y decide internamente cuántas veces llama a `getValor`:

```c
typedef void (*FuncionOperacion)(Vmx *vmx);

void add(Vmx *vmx) {
    uint8_t tipoA = getTipo(vmx->registros[OP1]);
    int32_t datoA = getDato(vmx->registros[OP1]);
    uint8_t tipoB = getTipo(vmx->registros[OP2]);
    int32_t datoB = getDato(vmx->registros[OP2]);

    int32_t a = getValor(vmx, tipoA, datoA);
    int32_t b = getValor(vmx, tipoB, datoB);
    int32_t res = a + b;

    actualizarCC(vmx, res);
    guardarRes(vmx, tipoA, datoA, res);
}

void not_(Vmx *vmx) {
    uint8_t tipoA = getTipo(vmx->registros[OP1]);
    int32_t datoA = getDato(vmx->registros[OP1]);
    int32_t a = getValor(vmx, tipoA, datoA);
    int32_t res = ~a;
    actualizarCC(vmx, res);
    guardarRes(vmx, tipoA, datoA, res);
}

void stop(Vmx *vmx) {
    vmx->registros[IP] = -1;
}

FuncionOperacion tabla_operaciones[32] = {0}; // huecos quedan en NULL
tabla_operaciones[OP_ADD]  = add;
tabla_operaciones[OP_NOT]  = not_;
tabla_operaciones[OP_STOP] = stop;
// ...

FuncionOperacion obtener_operacion(Vmx *vmx, uint8_t codigo) {
    return tabla_operaciones[codigo]; // NULL si el código es inválido — ya es la validación
}
```

En el ciclo de ejecución: `f = obtener_operacion(vmx, vmx->registros[OPC]); if (f == NULL) { /* error */ } else { f(vmx); }`.

**Se descartó** un enfoque alternativo donde el decodificador resolvía `getValor` antes de llamar al operador (pasándole los valores ya calculados en vez de tipo+dato): rompe la firma uniforme (necesitaría tablas separadas para 0, 1 y 2 operandos) y pierde la información de destino que `guardarRes` necesita.

**Detalle específico no cubierto por la especificación:** DIV guarda el resto en AC; RND genera un número aleatorio entre 0 y B guardándolo en A; la familia de saltos condicionales (JP, JN, JZ, JC, JV, JNP, JNN, JNZ) evalúa los bits N/Z/C/V del registro CC.

**Llamadas al sistema (SYS):** EAX indica READ (1) o WRITE (2). EDX contiene la posición de memoria inicial, y ECX la cantidad de celdas (2 bytes menos significativos) y el tamaño de cada una (2 bytes más significativos). Por cada dato accedido se debe mostrar en pantalla un prompt `[XXXX]:` con la dirección física en 4 dígitos hexadecimales.

### 5.8 Función JUMP centralizada

No se resuelve con recursividad (los saltos no vuelven a un estado anterior, todo es sucesivo). Existe una función JUMP única, ubicada fuera de los operadores (no es un operando en sí, pero todos los operandos de tipo salto la usan). `JMP` llama a JUMP directamente; `JP`, `JZ`, etc. la llaman agregando validaciones sobre los bits de CC.

### 5.9 Registro CC

Módulo/lógica encargada de actualizar los flags (N, Z, C, V) según la especificación, cada vez que corresponda (MOV, ADD, SUB, MUL, DIV, CMP, AND, OR, XOR, SWAP, SHL, SHR, SAR, NOT). Responsabilidad compartida por el equipo al implementar cada operador — quien programa un operador que afecta CC debe dejarlo actualizado. `actualizarCC` es una sola función compartida; lo que varía por operador es si se la llama o no, no su lógica interna.

### 5.10 Memoria (incluye la tabla de segmentos)

No es un módulo aparte de memoria — la tabla de segmentos solo sirve para validar/traducir direcciones:

```c
void validarPos(Vmx *vmx, uint16_t segmento, uint16_t pos_logica, uint16_t cantidad_bytes);
uint16_t traducirDireccion(Vmx *vmx, uint16_t segmento, uint16_t pos_logica);
int32_t getValorMemoria(Vmx *vmx, uint16_t pos_fisica, uint8_t cantidad_bytes);
void setValor(Vmx *vmx, uint16_t pos_fisica, uint8_t cantidad_bytes, int32_t valor);
```

### 5.11 Disassembler (`-d`)

Módulo distinto al decodificador (aunque puede reutilizarlo internamente). Cada línea se arma como **string en un buffer** (no `printf` directo), para que los tests puedan comparar el string esperado sin necesitar redirigir stdout:

```c
void obtener_linea_disassembler(Vmx *vmx, uint16_t direccion, char *buffer);
```

El orquestador del disassembler llama a esta función por cada instrucción del segmento de código e imprime una línea (`[dirección] XX XX XX XX | MNEM OP_A, OP_B`) por vez.

### 5.12 Validaciones y testing

Funciones validadoras comunes, separadas si hay validaciones que se repiten entre distintos procesos; si no hay reutilización real, van directamente en el flujo general. Testing: módulo aparte (ver sección 8).

## 6. Direcciones lógicas y físicas (por qué existe la indirección)

El programa no usa direcciones físicas "quemadas" en sus instrucciones porque el tamaño del segmento de código varía según cada programa traducido, y por lo tanto el segmento de datos empieza en una posición física distinta cada vez. Usando una **dirección lógica** (2 bytes código de segmento + 2 bytes offset), el binario siempre dice "offset N dentro de tal segmento", y es la VM la que en tiempo de ejecución consulta la tabla de descriptores (armada recién al cargar ese programa puntual) para traducir a dirección física.

Uso de LAR/MAR/MBR en cada acceso a memoria:

1. Cargar en **LAR** la dirección lógica a la que se quiere acceder.
2. Cargar en la parte alta de **MAR** (2 bytes más significativos) la cantidad de bytes a leer/escribir.
3. Traducir la dirección lógica a física usando la tabla de segmentos; el resultado va en la parte baja de **MAR** (2 bytes menos significativos).
4. **MBR** guarda el valor con el que se opera: el dato a escribir, o el dato leído.
5. La lectura de la instrucción en sí (para decodificar) no modifica estos registros.

## 7. Validaciones a implementar

1. Instrucción inválida (combinación de código de operación inexistente) → función de operación resuelve a `NULL` en la tabla.
2. Acceso a memoria inaccesible / fuera de los límites del segmento.
3. Acceso a registros reservados (posiciones 7-9, 18-25, 28-31 → **15 registros reservados**, no 17; de los 32 totales se usan 17).
4. Segmentos de la tabla de descriptores inválidos: código de segmento fuera de la tabla, o entrada con valor `-1` (`0xFFFFFFFF`).
5. División por cero.
6. Overflow / carry en operaciones aritméticas.
7. Límite de memoria alcanzado.
8. Cabecera del `.vmx` inválida (identificador o versión no soportada) al cargar el programa.
9. Longitud de código binario muy larga o muy corta.

**Manejo de errores**: ante cualquiera de estos casos, la VM debe informar el error por consola e **inmediatamente abortar la ejecución** del proceso (no solo detectarlo).

## 8. Testing

### 8.1 Traducción de tests (decidido)

Comando aparte, separado del comando que corre los tests: recorre los `.asm` de `tests/asm/`, compara fecha de modificación contra el `.vmx` correspondiente en `tests/vmx/` y solo retraduce los nuevos o modificados. El comando que corre los tests asume que los binarios ya están traducidos y solo los ejecuta. Se descarta traducir en cada corrida de test por el tiempo muerto que implica al crecer la cantidad de archivos.

### 8.2 Estructura de tests

- Módulo de testing separado, activable mediante un argumento de línea de comando específico que corra todos los tests.
- Los `.asm` de test viven en `tests/asm/`, agrupados por categoría (ej.: `tests/asm/mov/`, `tests/asm/jumps/`); los binarios traducidos en la carpeta equivalente `tests/vmx/`.
- Siguiendo la recomendación de la cátedra: cada archivo `.asm` contiene una única instrucción simple (ej.: `MOV EBX, ECX`) para los casos base; luego, tests de complejidad creciente que combinan varias instrucciones (incluyendo ejercicios completos, ej.: "el número ingresado es primo").
- Cada test es una **función de test en C**, con nombre significativo (idealmente igual al del archivo `.asm`/`.vmx` correspondiente), que:
    1. Ejecuta un binario puntual (nombre de archivo hardcodeado en el código — un archivo no encontrado ya produce un error obvio e inmediato, no hace falta ninguna capa de derivación automática del nombre).
    2. Valida con asserts los valores esperados: registros, memoria y/o salida por pantalla (incluyendo la salida del disassembler con `-d` cuando corresponda).
- Al menos un test por cada operación (MUL, MOV, etc.), más los necesarios para cubrir casos particulares.
- Tests agrupados en varios archivos por categoría (`tests_mov.c`, `tests_jumps.c`, `tests_aritmetica.c`, etc.), con un runner central que los llama a todos — evita un único archivo de tests con muchas líneas.
- Funciones auxiliares/helper para no repetir lógica en cada test: `ejecutar_binario`, `assert_registro`, `assert_memoria`, `assert_salida`, etc. Estas centralizan además el reporte de pass/no pass por nombre de test.

### 8.3 Generación de tests con IA

- La IA genera el código de los tests, **incluyendo el valor esperado de cada assert**; el equipo valida cada uno antes de darlo por bueno (Martin a cargo de este proceso).
- **Precaución según el tipo de valor esperado:**
    - Si el resultado esperado requiere precisión numérica exacta (ej.: el resultado puntual de una cuenta aritmética, un valor de registro tras varias operaciones), el equipo debe prestar atención extra o hacer el cálculo a mano antes de aceptar el test — la IA puede errar en el valor exacto.
    - En el caso general de tests que resuelven un ejercicio con un resultado fácil de verificar a simple vista (ej.: "el número ingresado es primo", true/false), el assert es simple de diseñar y de validar, y no requiere el mismo nivel de precaución.
- En todos los casos, la IA acelera la generación de volumen; la validación final es responsabilidad del equipo, no un paso opcional.

## 9. Pendientes / a definir más adelante

- Verificación empírica del layout exacto de bits del primer byte de instrucción (con `vmt` + hexdump), a cargo de Lucas — no es una decisión de diseño grupal, queda fuera de este documento.

## 10. Roadmap de desarrollo

Orden pensado por dependencias: cada etapa habilita a la siguiente, y dentro de una etapa las tareas se pueden repartir entre los 3 en paralelo.

### Etapa 0 — Verificación previa (bloqueante)

- Generar `.vmx` de ejemplo con `vmt` (dos operandos, un operando, sin operandos) y confirmar con hexdump el layout real del primer byte y el orden de los operandos en el binario (a cargo de Lucas). Nada del decodificador se escribe antes de esto.

### Etapa 1 — Esqueleto de la VM

- Definir el struct VMX (registros, memoria, tabla de segmentos) y sus funciones de creación/inicialización.
- Implementar el enum de registros y el array `int32_t registros[32]`.
- Implementar el array `uint8_t memoria[16384]` y `uint32_t tabla_segmentos[8]`.
- Parseo de argumentos de línea de comando (`vmx filename.vmx [-d]`).

### Etapa 2 — Carga del programa

- Lectura del archivo `.vmx` y validación de cabecera (identificador `"VMX26"`, versión soportada).
- Carga del código en memoria a partir de la posición 0.
- Armado de la tabla de segmentos (entrada 0 = código, entrada 1 = datos, resto en -1).
- Inicialización de registros: `CS`, `DS`, `IP = CS`.

### Etapa 3 — Direccionamiento de memoria

- Función de traducción de dirección lógica a física (usa LAR, MAR).
- Validaciones de fallo de segmento (código de segmento inválido, entrada -1, acceso fuera de límites).
- Funciones de lectura/escritura de memoria usando MAR/MBR.

### Etapa 4 — Decodificador de instrucciones

- Lectura del primer byte: extracción de tipos de operando y código de operación, según la cantidad de operandos que corresponda al código leído.
- Lectura de los operandos siguientes (orden inverso al Assembler, tal como se confirmó en la Etapa 0), con sign-extension para inmediatos y desplazamientos.
- Volcado de OPC, OP1, OP2 según el formato definido en la especificación (byte alto = tipo, resto = valor).

### Etapa 5 — Operadores y registro CC

- Array de punteros a función + enum de operaciones (`tabla_operaciones[32]`).
- Implementación de los operadores de dos operandos (MOV, ADD, SUB, MUL, DIV, CMP, AND, OR, XOR, SWAP, SHL, SHR, SAR, LDL, LDH, RND).
- Implementación de NOT y de la función JUMP centralizada, junto con JMP y la familia de saltos condicionales (JP, JN, JZ, JC, JV, JNP, JNN, JNZ).
- Implementación de STOP.
- Lógica de actualización del registro CC (N, Z, C, V) integrada en cada operador que corresponda.

### Etapa 6 — Ciclo de ejecución completo

- Ciclo fetch-decode-execute: leer instrucción por IP, decodificar, avanzar IP, ejecutar.
- Condición de corte: IP fuera del segmento de código, o IP = -1 tras STOP.
- Manejo de errores con corte inmediato: instrucción inválida, división por cero, fallo de segmento.

### Etapa 7 — Llamadas al sistema (SYS)

- Implementación de READ y WRITE usando EAX, EDX, ECX según la especificación.
- Formato de salida por pantalla con el prompt `[XXXX]` de dirección física.

### Etapa 8 — Disassembler (`-d`)

- Módulo aparte que reutiliza el decodificador para traducir el binario cargado a texto legible.
- Formato de salida: `[dirección] XX XX XX XX | MNEM OP_A, OP_B`.

### Etapa 9 — Testing

- Comando de traducción con cacheo (Etapa 8.1 del documento, sección 8.1).
- Primeros tests: uno por cada operador simple, agrupados por categoría.
- Tests de complejidad creciente combinando instrucciones (incluyendo ejercicios completos tipo guía).
- Generación asistida por IA con validación del equipo, según lo definido en la sección 8.3.

### Etapa 10 — Repaso y entrega

- Revisión cruzada del código completo entre los 3.
- Verificación de que todas las validaciones de la sección 7 estén cubiertas por al menos un test.
- Preparación de la entrega (código fuente + ejecutable compilado).