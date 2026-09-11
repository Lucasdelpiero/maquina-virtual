# Apuntes de diseño — Máquina Virtual (vmx) — Plan de Implementación 5

_Fundamentos de la Arquitectura de Computadoras — UNMDP — MV1 2026_

---

## Índice

- [1. Contexto del proyecto](#1-contexto-del-proyecto)
- [2. Estructura recomendada del proyecto y patrón arquitectónico](#2-estructura-recomendada-del-proyecto-y-patrón-arquitectónico)
  - [2.1 ¿Existe un patrón de arquitectura para este proyecto?](#21-existe-un-patrón-de-arquitectura-para-este-proyecto)
  - [2.2 Organización de carpetas: ¿Una carpeta por cada módulo?](#22-organización-de-carpetas-una-carpeta-por-cada-módulo)
  - [2.3 Árbol de directorios recomendado](#23-árbol-de-directorios-recomendado)
- [3. Equipo y roles](#3-equipo-y-roles)
- [4. Principios de diseño y convenciones](#4-principios-de-diseño-y-convenciones)
- [5. Estructuras de datos centrales](#5-estructuras-de-datos-centrales)
  - [5.1 Registros](#51-registros)
  - [5.2 Operaciones y tabla de dispatch](#52-operaciones-y-tabla-de-dispatch)
  - [5.3 Memoria principal y tabla de segmentos](#53-memoria-principal-y-tabla-de-segmentos)
- [6. Módulos y arquitectura del sistema](#6-módulos-y-arquitectura-del-sistema)
  - [6.1 Struct central `Vmx` (TDA)](#61-struct-central-vmx-tda)
  - [6.2 Módulo Logger (`logger.h` / `logger.c`)](#62-módulo-logger-loggerh--loggerc)
  - [6.3 Módulo Manejo de Errores y Abortar (`errores.h` / `errores.c`)](#63-módulo-manejo-de-errores-y-abortar-erroresh--erroresc)
  - [6.4 Módulo Inicializador (`inicializador.h` / `inicializador.c`)](#64-módulo-inicializador-inicializadorh--inicializadorc)
  - [6.5 Módulo Memoria y Traducción (`memoria.h` / `memoria.c`)](#65-módulo-memoria-y-traducción-memoriah--memoriac)
  - [6.6 Módulo Decodificador (`decodificador.h` / `decodificador.c`)](#66-módulo-decodificador-decodificadorh--decodificadorc)
  - [6.7 Módulo Operandos (`operandos.h` / `operandos.c`)](#67-módulo-operandos-operandosh--operandosc)
  - [6.8 Módulo Operadores y Código de Condición (`operadores.h` / `operadores.c`)](#68-módulo-operadores-y-código-de-condición-operadoresh--operadoresc)
  - [6.9 Módulo Llamadas al Sistema (`sys.h` / `sys.c`)](#69-módulo-llamadas-al-sistema-sysh--sysc)
  - [6.10 Módulo Disassembler (`disassembler.h` / `disassembler.c`)](#610-módulo-disassembler-disassemblerh--disassemblerc)
  - [6.11 Módulo Principal (`main.c`)](#611-módulo-principal-mainc)
- [7. Validaciones y manejo de errores](#7-validaciones-y-manejo-de-errores)
- [8. Estrategia de testing y verificación](#8-estrategia-de-testing-y-verificación)
  - [8.1 Organización de la suite de tests](#81-organización-de-la-suite-de-tests)
  - [8.2 Casos de prueba prioritarios](#82-casos-de-prueba-prioritarios)
- [9. Roadmap de implementación](#9-roadmap-de-implementación)

---

## 1. Contexto del proyecto

El trabajo consiste en implementar, en lenguaje C (estándar C99 / C clásico), la **máquina virtual (vmx)** que ejecuta programas ya traducidos a lenguaje máquina (`.vmx`). La traducción de Assembler (`.asm`) a binario la realiza el **traductor `vmt`, provisto por la cátedra** — no es parte de lo que construimos.

Por línea de comando, a la VM se le pasa el **nombre del archivo `.vmx`** a ejecutar y flags opcionales:

```bash
vmx filename.vmx [-d] [-dev]
```

- `-d`: activa el disassembler para mostrar por pantalla la desensamblación del código máquina (ver sección 6).
- `-dev` o `--debug`: activa el modo desarrollador con trazas del logger interno para depuración.

---

## 2. Estructura recomendada del proyecto y patrón arquitectónico

### 2.1 ¿Existe un patrón de arquitectura para este proyecto?
Sí, el patrón que rige este tipo de desarrollos en C es la **Arquitectura Modular basada en TDA (Tipos de Datos Abstractos)** modelada a partir de los subsistemas del procesador (arquitectura von Neumann). No se aplican patrones de software empresarial (como MVC o capas de negocio), sino una separación de responsabilidades dictada por el hardware emulado:

1. **Subsistema de Almacenamiento y Bus**: `memoria.c/.h` (RAM de 16 KiB, Descriptores de Segmentos, buses LAR/MAR/MBR) y `registros.c/.h`.
2. **Subsistema de CPU (Pipeline Fetch-Decode-Execute)**:
   - `decodificador.c/.h`: lectura del binario, extracción de opcode y operandos.
   - `operandos.c/.h`: direccionamiento lógico/físico, resolución de valores y `combinarMitad`.
   - `operadores.c/.h`: ejecución de las 26 operaciones, cálculo de CC (N, Z, C, V) y saltos.
3. **Subsistema de Orquestación y Máquina Virtual**: `vmx.c/.h` (struct central `Vmx`, ciclo de ejecución `while (ipValido)`) e `inicializador.c/.h`.
4. **Subsistema de E/S y Diagnóstico**: `sys.c/.h` (llamadas al sistema), `disassembler.c/.h` (flag `-d`) y `logger.c/.h` (trazas y abortar).

### 2.2 Organización de carpetas: ¿Una carpeta por cada módulo?
**No se recomienda crear una carpeta individual por cada módulo** (por ejemplo, evitar `decodificador/decodificador.c`, `memoria/memoria.c`, etc.):
- **Sobrecarga de includes**: Obliga a escribir includes largos (`#include "../memoria/memoria.h"`) o a configurar múltiples rutas de inclusión en el compilador.
- **Sobre-ingeniería para el tamaño del proyecto**: Con 8 a 10 módulos, crear una carpeta para cada par `.c`/`.h` fragmenta la navegación sin aportar beneficios.

### 2.3 Árbol de directorios recomendado

La estructura estándar, limpia y sin requerimientos de configuración extra en el IDE es consolidar los módulos (`.c` y `.h` juntos) en la carpeta `src/`:

```text
proyecto-maquina-virtual/
│
├── docs/                                # Documentación, especificaciones y planes
│   └── martin/
│       ├── Especificacion (pdf a .md).md
│       ├── Lenguaje Assembler Parte 1.md
│       ├── Plan de Impl 5.md
│       └── ...
│
├── src/                                 # Módulos de la VM (.c y .h juntos)
│   ├── vmx.h                            # Definición central del struct Vmx y enums
│   ├── vmx.c                            # Ciclo de ejecución fetch-decode-execute
│   ├── inicializador.h
│   ├── inicializador.c                  # inicializarVmx y cargarPrograma
│   ├── memoria.h
│   ├── memoria.c                        # RAM, tabla segmentos, LAR, MAR, MBR
│   ├── decodificador.h
│   ├── decodificador.c                  # Parseo de instrucciones
│   ├── operandos.h
│   ├── operandos.c                      # getTipo, getValor, guardarRes, combinarMitad
│   ├── operadores.h
│   ├── operadores.c                     # Las 26 instrucciones y actualización CC
│   ├── sys.h
│   ├── sys.c                            # Syscalls: READ y WRITE
│   ├── disassembler.h
│   ├── disassembler.c                   # Formateo de código desensamblado (-d)
│   ├── logger.h
│   ├── logger.c                         # Trazas de depuración (-dev)
│   ├── errores.h
│   └── errores.c                        # Función abortar (salida fatal)
│
├── tests/                               # Suite de testing automatizada
│   ├── asm/                             # Programas .asm de prueba por categoría
│   │   ├── mov/
│   │   ├── aritmetica/
│   │   └── saltos/
│   ├── vmx/                             # Binarios .vmx generados con vmt.exe
│   └── src/                             # Código C de pruebas unitarias
│       ├── tests_runner.c               # main() del ejecutable de pruebas
│       ├── tests_aritmetica.c
│       ├── tests_mov.c
│       └── tests_saltos.c
│
├── main.c                               # Punto de entrada de la aplicación normal
├── README.md                            # Documento principal del repositorio
└── .gitignore                           # Exclusión de binarios y temporales
```

> **Ventaja de diseño**: Mantener los archivos `.h` y `.c` juntos en `src/` permite que `#include "memoria.h"` funcione directamente sin necesidad de configurar rutas de búsqueda (*Search directories*) adicionales en los entornos de desarrollo del equipo. En el explorador de archivos, cada módulo queda agrupado ordenadamente con su cabecera e implementación lado a lado.

---

## 3. Equipo y roles

Equipo de 3 personas. **Los 3 son arquitectos de la solución**: diseño, revisión de código e implementación general son responsabilidad compartida.

- **Renzo, Lucas y Martin**: diseño de solución, code review general, implementaciones de arquitectura.
- **Renzo**: desarrollo de operaciones específicas y funciones auxiliares de texto para el disassembler (por ejemplo, mnemónicos de instrucciones).
- **Lucas**: verificación empírica de formatos binarios, hexdump y análisis de compatibilidad con `vmt`.
- **Martin**: armado de la infraestructura de tests automatizados y generación asistida por IA con posterior validación técnica.

Regla de trabajo: una persona por archivo a la vez; coordinación mediante ramas de Git evitando commits directos sobre la rama principal.

---

## 4. Principios de diseño y convenciones

- **Lenguaje**: C estándar clásico/básico (C99), legible y estructurado.
- **Convención de nomenclatura**:
  - **camelCase** unificado para nombres de funciones y variables: `obtenerOperacion`, `tablaOperaciones`, `ipValido`, `leerInstruccion`, `actualizarCC`, `posFisica`, `codReg`, etc.
  - **MAYÚSCULAS_CON_GUION_BAJO** para constantes, macros y elementos de enums: `OP_MOV`, `EAX`, `CS`, `TIPO_MEMORIA`, etc.
  - Los mnemónicos y registros se mantienen estrictamente con los nombres provistos por la cátedra (`MOV`, `ADD`, `IP`, `CC`, `DS`, etc.).
- **Diseño basado en TDA**: structs que representan entidades (`Vmx`, etc.) pasados explícitamente por puntero (`Vmx *vmx`).
- **Sin variables globales (`extern`)**: todo el estado reside en la instancia del struct `Vmx`.
- **Simulación de métodos mediante punteros a funciones en el struct**: para operaciones centrales como `vmx->abortar(vmx, mensaje)` y `vmx->logger(vmx, mensaje)`.
- **Tipos de tamaño fijo (`stdint.h`)**:
  - `int32_t`: para valores con signo del dominio (registros de propósito general, datos numéricos de cálculo, código de condición).
  - `int16_t`: para desplazamientos con signo (**offsets** en operandos de memoria, rango de -32768 a 32767).
  - `uint8_t`: para celdas crudas de memoria física, opcodes, tipos de operandos.
  - `uint16_t`: para magnitudes sin signo como direcciones físicas (0 a 16383) y tamaños de segmentos (1 a 16384).
  - `uint32_t`: para entradas empaquetadas de la tabla de descriptores de segmentos (`base:16 | tamaño:16`).
  - `int64_t` / `uint64_t`: para cálculos intermedios de detección exacta de carry y overflow sin desbordamiento espurio.

---

## 5. Estructuras de datos centrales

### 5.1 Registros

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

- Hay 17 registros en uso y 15 reservados en esta primera etapa.
- Indexación dinámica directa por código numérico de 0 a 31 (`registros[codigo]`) sin requerir cadenas de `switch`.

### 5.2 Operaciones y tabla de dispatch

```c
typedef enum {
    OP_SYS = 0x00, OP_JMP = 0x01, OP_JP = 0x02, OP_JN = 0x03, OP_JZ = 0x04,
    OP_JC = 0x05, OP_JV = 0x06, OP_JNP = 0x07, OP_JNN = 0x08, OP_JNZ = 0x09,
    OP_NOT = 0x0A,
    // 0x0B a 0x0E: sin uso / reservados
    OP_STOP = 0x0F,
    OP_MOV = 0x10, OP_ADD = 0x11, OP_SUB = 0x12, OP_MUL = 0x13, OP_DIV = 0x14,
    OP_CMP = 0x15, OP_AND = 0x16, OP_OR = 0x17, OP_XOR = 0x18, OP_SWAP = 0x19,
    OP_SHL = 0x1A, OP_SHR = 0x1B, OP_SAR = 0x1C, OP_LDL = 0x1D, OP_LDH = 0x1E,
    OP_RND = 0x1F
} CodigoOperacion;

typedef struct Vmx Vmx;
typedef void (*FuncionOperacion)(Vmx *vmx);

FuncionOperacion tablaOperaciones[32]; // Inicializada en NULL
```

- Despacho directo: `FuncionOperacion op = tablaOperaciones[vmx->registros[OPC]];`
- Si `op == NULL`, se trata de una instrucción inválida y se invoca inmediatamente `vmx->abortar(vmx, "Instrucción inválida");`.

### 5.3 Memoria principal y tabla de segmentos

```c
uint8_t memoria[16384]; // 16 KiB direccionables byte a byte
uint32_t tablaSegmentos[8]; // Entradas empaquetadas: (base << 16) | tamano
```

- Cada descriptor de segmento empaqueta:
  - Base: 16 bits más significativos.
  - Tamaño: 16 bits menos significativos.
  - Valor no inicializado / inválido: `0xFFFFFFFF` (`-1`).

---

## 6. Módulos y arquitectura del sistema

### 6.1 Struct central `Vmx` (TDA)

```c
struct Vmx {
    int32_t registros[32];
    uint8_t memoria[16384];
    uint32_t tablaSegmentos[8];
    int modoDebug;        // Flag para trazas de depuración (-dev)
    int modoDisassembler; // Flag para ejecución con disassembler (-d)

    // Punteros a funciones de utilidad
    void (*logger)(Vmx *vmx, const char *mensaje);
    void (*abortar)(Vmx *vmx, const char *mensaje);
};
```

> **Nota sobre includes**: El struct `Vmx` no necesita incluir `logger.h` ni `errores.h`. Solo declara los punteros a función con la firma genérica `(Vmx *vmx, const char *mensaje)`. Quien sí incluye `logger.h` y `errores.h` es `inicializador.c` para enlazar los punteros con sus implementaciones reales.

### 6.2 Módulo Logger (`logger.h` / `logger.c`)
Módulo dedicado exclusivamente a la emisión de trazas de desarrollo y depuración:
- `logMsg(Vmx *vmx, const char *mensaje)`: imprime el mensaje por consola con prefijo `[LOG]` únicamente si `vmx->modoDebug == 1`. Si el flag está en 0, no produce salida.

### 6.3 Módulo Manejo de Errores y Abortar (`errores.h` / `errores.c`)
Módulo dedicado al control de fallos fatales e interrupción inmediata del proceso:
- `abortar(Vmx *vmx, const char *mensaje)`: imprime el mensaje de error por salida estándar de error / consola y finaliza la ejecución inmediatamente mediante `exit(EXIT_FAILURE)`.

### 6.4 Módulo Inicializador (`inicializador.h` / `inicializador.c`)
Separa claramente la inicialización del sistema en dos fases bien definidas:
1. **Inicialización intrínseca de la máquina (`inicializarVmx`)**:
   - Limpia en cero registros, memoria y tabla de descriptores.
   - Enlaza `vmx->logger = logMsg;` y `vmx->abortar = abortar;`.
   - Inicializa las entradas 2 a 7 de la tabla de descriptores en `0xFFFFFFFF`.
   - Inicializa el array `tablaOperaciones` con los punteros a cada función de operación.
2. **Carga y configuración del programa (`cargarPrograma`)**:
   - Abre el archivo binario con `fopen(rutaArchivo, "rb")`. Si falla, invoca `vmx->abortar(vmx, "No se pudo abrir el archivo .vmx");`.
   - Lee los 8 bytes de cabecera de una sola vez con `fread(cabecera, 1, 8, archivo)`.
   - Valida el identificador (bytes 0-4: `"VMX26"`) y la versión (byte 5: `1`). Si no coinciden, aborta.
   - Extrae el tamaño del código combinando los bytes 6 y 7 en formato big-endian:
     `uint16_t tamanoCodigo = (cabecera[6] << 8) | cabecera[7];`
   - Valida que `tamanoCodigo > 0` y que `tamanoCodigo <= 16384`.
   - Carga el código completo en la memoria física a partir de la posición 0 de una sola vez:
     `fread(&vmx->memoria[0], 1, tamanoCodigo, archivo);`
   - Cierra el archivo con `fclose(archivo);`.
   - Configura la tabla de descriptores de segmentos:
     - Entrada 0 (Código): Base = 0, Tamaño = `tamanoCodigo`.
     - Entrada 1 (Datos): Base = `tamanoCodigo`, Tamaño = `16384 - tamanoCodigo`.
   - Inicializa los registros puntero (direcciones lógicas):
     - `CS = 0x00000000` (código de segmento 0, offset 0).
     - `DS = 0x00010000` (código de segmento 1, offset 0).
     - `IP = CS`.

### 6.5 Módulo Memoria y Traducción (`memoria.h` / `memoria.c`)
Gestiona el acceso seguro a la memoria principal y el manejo de los registros de bus `LAR`, `MAR`, `MBR`:
- `validarAcceso(Vmx *vmx, uint16_t codSegmento, int16_t offset, uint16_t cantBytes)`:
  - Comprueba que `codSegmento < 8`.
  - Comprueba que la entrada no sea `0xFFFFFFFF`.
  - Extrae `base` y `tamano`.
  - Verifica que `(offset + cantBytes) <= tamano`. Si falla, invoca `vmx->abortar(vmx, "Fallo de segmento: acceso fuera de límites");`.
- `traducirDireccion(Vmx *vmx, int32_t dirLogica, uint16_t cantBytes)`:
  - Carga `LAR = dirLogica`.
  - Carga la parte alta de `MAR` con `cantBytes`.
  - Valida el segmento y calcula la dirección física: `dirFisica = base + offset`.
  - Carga la parte baja de `MAR` con `dirFisica`.
  - Retorna `dirFisica`.
- `leerMemoria(Vmx *vmx, uint16_t dirFisica, uint8_t cantBytes)`:
  - Lee 1, 2 o 4 bytes (en formato big-endian).
  - Carga el valor leído en `MBR` y lo retorna.
- `escribirMemoria(Vmx *vmx, uint16_t dirFisica, uint8_t cantBytes, int32_t valor)`:
  - Carga `MBR = valor`.
  - Escribe los bytes correspondientes en la memoria física.

### 6.6 Módulo Decodificador (`decodificador.h` / `decodificador.c`)
Decodifica la instrucción apuntada por `IP` dentro del segmento de código sin modificar los registros `LAR`/`MAR`/`MBR`:
1. Verifica que `IP` apunte dentro del segmento de código (Base=0, Límite=`tamanoCodigo`). Si `IP == -1` (`0xFFFFFFFF`), finaliza la ejecución normal. Si se sale de rango sin STOP, aborta por error.
2. Lee el primer byte de la instrucción:
   - Extrae el código de operación (5 bits menos significativos): `opc = primerByte & 0x1F`.
   - Según el `opc`, determina la cantidad de operandos (0, 1 o 2).
   - Extrae los tipos de operando (00=ninguno, 01=registro, 10=inmediato, 11=memoria).
3. Lee los operandos del binario:
   - **Orden de codificación verificado**: En instrucciones de dos operandos, el binario codifica primero el operando B y luego el operando A.
   - Realiza la extensión de signo de 16 a 32 bits de forma manual mediante una función auxiliar explícita:
     ```c
     int32_t extenderSigno16a32(uint16_t valor16) {
         // Si el bit 15 (signo) es 1, es negativo: completamos los 16 bits altos con 1s (0xFFFF0000)
         if (valor16 & 0x8000) {
             return (int32_t)(valor16 | 0xFFFF0000);
         }
         // Si es positivo, los 16 bits altos quedan en ceros
         return (int32_t)valor16;
     }
     ```
4. Empaqueta y almacena en los registros:
   - `vmx->registros[OPC] = opc`.
   - `vmx->registros[OP1] = (tipoA << 24) | (datoA & 0x00FFFFFF)`.
   - `vmx->registros[OP2] = (tipoB << 24) | (datoB & 0x00FFFFFF)`.
5. Avanza el registro `IP` sumando la cantidad de bytes consumidos por la instrucción actual.

### 6.7 Módulo Operandos (`operandos.h` / `operandos.c`)
Encapsula la extracción y resolución de operandos:
- `obtenerTipo(int32_t operando)`: retorna `(operando >> 24) & 0xFF`.
- `obtenerDato(int32_t operando)`: retorna el valor de 24 bits con su correspondiente signo.
- `resolverDireccionMemoria(Vmx *vmx, int32_t dato)`:
  - `codReg = dato & 0x1F`.
  - `uint16_t offsetCrudo = (dato >> 8) & 0xFFFF`.
  - `int32_t offset = extenderSigno16a32(offsetCrudo)`.
  - Toma el registro base `vmx->registros[codReg]`.
  - Invoca `traducirDireccion(vmx, base + offset, 4)`.
- `obtenerValor(Vmx *vmx, uint8_t tipo, int32_t dato)`:
  - Si es registro: `vmx->registros[dato]`.
  - Si es inmediato: `dato`.
  - Si es memoria: `leerMemoria(vmx, posFisica, 4)`.
- `guardarResultado(Vmx *vmx, uint8_t tipo, int32_t dato, int32_t valor)`:
  - Si es registro: `vmx->registros[dato] = valor`.
  - Si es memoria: `escribirMemoria(vmx, posFisica, 4, valor)`.
  - Si es inmediato: `vmx->abortar(vmx, "Error: intento de escribir en operando inmediato");`.
- `combinarMitad(Vmx *vmx, uint8_t tipo, int32_t dato, uint16_t mitadNueva, int cargarAlta)`:
  - Función especial para `LDH` y `LDL`. Realiza un ciclo *read-modify-write*: lee los 32 bits actuales mediante `obtenerValor`, combina los 16 bits nuevos en la parte alta o baja preservando los otros 16 bits, y guarda los 32 bits completos mediante `guardarResultado`.

### 6.8 Módulo Operadores y Código de Condición (`operadores.h` / `operadores.c`)
Cada operador implementa la firma uniforme `void op(Vmx *vmx)`:

#### Actualización del Registro CC:
El registro `CC` se organiza según la especificación: `[N][Z][C][V][28 bits reservados]`.
- **N (Bit 31)**: 1 si el resultado de 32 bits con signo es menor a 0.
- **Z (Bit 30)**: 1 si el resultado de 32 bits es igual a 0.
- **C (Bit 29)**: Bit de acarreo / carry.
- **V (Bit 28)**: Bit de desbordamiento / overflow con signo.

#### Reglas de operaciones sobre flags:
1. **Operaciones que modifican CC (13 operaciones taxativas)**:
   - `MOV`, `ADD`, `SUB`, `MUL`, `DIV`, `CMP`, `AND`, `OR`, `XOR`, `SWAP`, `SHL`, `SHR`, `SAR`, `NOT`.
2. **Operaciones que NUNCA modifican CC**:
   - `RND`, `LDH`, `LDL`, `JMP`, `JP`, `JN`, `JZ`, `JC`, `JV`, `JNP`, `JNN`, `JNZ`, `SYS`, `STOP`.
3. **Significado y cálculo simple de Carry (C) y Overflow (V)**:
   - **Carry (C)**: Se activa cuando el resultado excede los 32 bits disponibles en cálculo sin signo (acarreo hacia el bit 33).
   - **Overflow (V)**: Se activa cuando el resultado desborda el rango de enteros con signo de 32 bits (`INT32_MIN` a `INT32_MAX`), por ejemplo al sumar dos números positivos grandes y que el bit de signo cambie erróneamente a negativo.
   - **Cálculo con constantes estándar (`<stdint.h>`)**: En lugar de usar números mágicos hardcodeados, usamos las constantes estándar del sistema (`INT32_MIN`, `INT32_MAX`, `UINT32_MAX`) para máxima claridad:
     - Para `ADD`:
       ```c
       int64_t sumaConSigno = (int64_t)a + (int64_t)b;
       int flagV = (sumaConSigno < INT32_MIN || sumaConSigno > INT32_MAX);

       uint64_t sumaSinSigno = (uint64_t)(uint32_t)a + (uint64_t)(uint32_t)b;
       int flagC = (sumaSinSigno > UINT32_MAX);
       ```
     - Para `SUB` y `CMP`:
       ```c
       int64_t restaConSigno = (int64_t)a - (int64_t)b;
       int flagV = (restaConSigno < INT32_MIN || restaConSigno > INT32_MAX);
       int flagC = ((uint32_t)a < (uint32_t)b); // Hubo préstamo / borrow
       ```
     - Para `MUL`:
       ```c
       int64_t prodConSigno = (int64_t)a * (int64_t)b;
       int flagV = (prodConSigno < INT32_MIN || prodConSigno > INT32_MAX);
       uint64_t prodSinSigno = (uint64_t)(uint32_t)a * (uint64_t)(uint32_t)b;
       int flagC = (prodSinSigno > UINT32_MAX);
       ```
     - Para `DIV`: Guarda el cociente en el destino y el resto en `AC`. **`DIV` siempre pone `C=0` y `V=0`** (matemáticamente el cociente de enteros nunca excede al dividendo). Si el divisor es 0, invoca `vmx->abortar(vmx, "Error: División por cero");`.
     - Para `SHL`: Se corre a la izquierda. Si se "cae" un bit 1 fuera de los 32 bits, se activa `C=1`. `V=1` si el bit de signo cambia.
     - Para `SHR` / `SAR`: Si el último bit expulsado hacia la derecha es 1, se activa `C=1`.
     - Para `AND`, `OR`, `XOR`, `NOT`: Actualizan `N` y `Z`; `C=0` y `V=0`.
     - Para `SWAP`: Intercambia operandos (permite memoria-memoria) y actualiza `CC` de la misma manera que `XOR`.

#### Saltos y función `ejecutarSalto`:
- `ejecutarSalto(Vmx *vmx, int32_t offsetDestino)`: actualiza `IP = vmx->registros[CS] + offsetDestino`.
- `opJmp`: llama directamente a `ejecutarSalto`.
- `opJp`, `opJn`, `opJz`, `opJc`, `opJv`, `opJnp`, `opJnn`, `opJnz`: evalúan los bits de `CC` según la tabla de la especificación y llaman a `ejecutarSalto` si la condición se cumple.
- `opStop`: fija `vmx->registros[IP] = -1` (`0xFFFFFFFF`), cortando el ciclo de ejecución.

### 6.9 Módulo Llamadas al Sistema (`sys.h` / `sys.c`)
Implementa las llamadas al sistema requeridas:
- `EAX`: Modo de operación (máscara de bits: bit 0=decimal, bit 1=caracteres, bit 2=octal, bit 3=hexadecimal, bit 4=binario).
- `EDX`: Dirección lógica inicial donde se leen o escriben los datos.
- `ECX`: Cantidad de valores (2 bytes menos significativos) y tamaño de cada valor en bytes (2 bytes más significativos).
- **SYS 1 (READ)**: Muestra el prompt `[XXXX]: ` con la dirección física en 4 dígitos hexadecimales y lee desde teclado en el formato pedido.
- **SYS 2 (WRITE)**: Imprime el prompt `[XXXX]: ` y formatea el contenido en los modos activados simultáneamente en `EAX`.

### 6.10 Módulo Disassembler (`disassembler.h` / `disassembler.c`)
- **Obtención de mnemónicos**: Una función (o array indexado por opcode) `const char* obtenerMnemonico(uint8_t opc)` devuelve el string correspondiente (ej: `0x10` -> `"MOV"`, `0x11` -> `"ADD"`). Esta función la desarrolla Renzo.
- **Formateo de línea**: El disassembler toma la instrucción decodificada y arma la línea en un buffer usando `snprintf`:
  `[dirección física en 4 hex] bytes_en_hex | MNEM OP_A, OP_B`
  (ejemplo: `[0000] B1 00 0A 00 05 9B | ADD [DS+5], 10`).
- **Buffer para testing**: Devuelve la línea armada en un buffer `char buffer[128]` antes de imprimirla, lo que permite que los tests unitarios de Martin comparen el texto resultante con `assert` de forma directa sin depender de la consola.

### 6.11 Módulo Principal (`main.c`)
Punto de entrada minimalista, orquestador sin lógica de bajo nivel dispersa:
```c
int main(int argc, char *argv[]) {
    // 1. Parsear argumentos de línea de comandos (archivo.vmx, -d, -dev)
    // 2. inicializarVmx(&vmx, modoDebug, modoDisassembler);
    // 3. cargarPrograma(&vmx, rutaArchivo);
    // 4. ejecutarVmx(&vmx);
    // 5. return 0;
}
```

---

## 7. Validaciones y manejo de errores

Ante cualquiera de las siguientes fallas, la máquina virtual emite un mensaje descriptivo por consola e **inmediatamente aborta la ejecución** mediante `vmx->abortar`:
1. **Archivo inaccesible o inexistente**.
2. **Cabecera de binario inválida**: identificador distinto de `"VMX26"` o versión no compatible.
3. **Tamaño de código inválido**: menor a 1 byte o mayor a los 16 KiB de memoria total.
4. **Instrucción inválida**: código de operación inexistente o apuntando a entrada `NULL` en `tablaOperaciones`.
5. **Acceso a registro reservado**: intento de direccionamiento sobre códigos de registro fuera de los 17 permitidos (7-9, 18-25, 28-31).
6. **Fallo de segmento**: código de segmento mayor o igual a 8, entrada no asignada (`0xFFFFFFFF`), o dirección que sobrepasa el límite del segmento (`offset + bytes > tamano`).
7. **División por cero**: segundo operando de `DIV` igual a 0.
8. **Destino inválido**: intento de escribir sobre un operando de tipo inmediato.

---

## 8. Estrategia de testing y verificación

### 8.1 Organización de la suite de tests
- **Ejecutable runner independiente**: `tests_runner` compila todos los casos de prueba y los ejecuta de forma secuencial.
- **Automatización de traducción (`vmt.exe`)**: Se incluye un script/herramienta de automatización (generado con IA) que recorre recursivamente todos los archivos `.asm` dentro de `tests/asm/`, ejecuta `vmt.exe` sobre cada uno y deposita los binarios `.vmx` correspondientes en `tests/vmx/`, traduciendo solo los archivos nuevos o modificados para agilizar las corridas.
- **Directorio de pruebas**:
  - `tests/asm/`: código fuente `.asm` clasificado por categorías (`mov/`, `aritmetica/`, `saltos/`, `memoria/`, `sys/`).
  - `tests/vmx/`: binarios `.vmx` generados automáticamente por el traductor `vmt.exe`.
  - `tests/src/`: código C de las suites de test (`tests_mov.c`, `tests_aritmetica.c`, `tests_saltos.c`, `tests_runner.c`).
- **Helpers de aserción**: `assertRegistro(vmx, reg, esperado)`, `assertMemoria(vmx, dir, esperado)`, `assertCC(vmx, n, z, c, v)`.

### 8.2 Casos de prueba prioritarios
1. **Operaciones aritméticas y flags**:
   - Casos de prueba de la especificación:
     - `MOV EAX, -1` -> N=1, Z=0, C=0, V=0.
     - `ADD 2147483647, 2147483647` -> res=-2, N=1, Z=0, C=0, V=1.
     - `SUB -3, 5` -> res=-8, N=1, Z=0, C=1, V=0.
     - `MUL 1073741827, 4` -> res=12, N=0, Z=0, C=1, V=1.
     - `DIV 6, 7` -> res=0, resto en AC=6, N=0, Z=1, C=0, V=0.
     - `SHL 1073741824, 2` -> res=0, N=0, Z=1, C=1, V=1.
2. **Read-Modify-Write**:
   - Pruebas unitarias de `LDH` y `LDL` verificando que preservan la mitad no modificada de registros y posiciones de memoria.
3. **Control de flujo y saltos**:
   - `JMP` incondicional.
   - Saltos condicionales (`JZ`, `JNZ`, `JP`, `JN`, `JNN`, `JNP`) según combinaciones de flags en `CC`.
   - `STOP` fijando `IP = -1` y terminación limpia.
4. **Protección de memoria y excepciones**:
   - Acceso fuera de límites de segmento provocando llamada a `abortar`.
   - División por cero abortando con mensaje correspondiente.

---

## 9. Roadmap de implementación

### Etapa 0 — Verificación previa del binario (COMPLETADA)
- Confirmada la estructura de cabecera `"VMX26"`, versión `1` y tamaño de 2 bytes big-endian.
- Confirmado el orden inverso de operandos en el binario (B luego A).

### Etapa 1 — Esqueleto de la VM y tipos base
- Definir `vmx.h` con el struct `Vmx`, enums de registros y operaciones.
- Implementar `errores.h` / `errores.c` con la función `abortar`.
- Implementar `logger.h` / `logger.c` con la función `logMsg`.

### Etapa 2 — Módulo Inicializador y Carga de Programa
- Implementar `inicializarVmx` (limpieza, conexión de métodos de struct).
- Implementar `cargarPrograma` (apertura de `.vmx`, validación de cabecera y armado de tabla de descriptores para código y datos).

### Etapa 3 — Memoria, Direccionamiento y Registros de Bus
- Implementar `validarAcceso`, `traducirDireccion`, `leerMemoria`, `escribirMemoria` manipulando `LAR`, `MAR`, `MBR`.
- Validar fallos de segmento.

### Etapa 4 — Decodificador de Instrucciones
- Implementar lectura de opcode, tipos y operandos con extensión de signo.
- Empaquetar en `OPC`, `OP1`, `OP2` y avanzar `IP`.

### Etapa 5 — Operandos y Operadores Centrales
- Implementar `operandos.c` (`obtenerTipo`, `obtenerDato`, `obtenerValor`, `guardarResultado`, `combinarMitad`).
- Implementar `actualizarCC` con cálculo de 64 bits para carry y overflow.
- Implementar los 16 operadores de dos operandos (incluyendo `LDH`, `LDL`, `SWAP`, `DIV` con `AC`).
- Implementar operadores de un operando (`NOT`, `JMP`, familia `Jcc`, `SYS`) y `STOP`.
- Conectar la tabla de despacho `tablaOperaciones[32]`.

### Etapa 6 — Ciclo de Ejecución Principal y Main
- Implementar `ejecutarVmx`: bucle `while (ipValido(vmx))` con fetch, decode y dispatch.
- Implementar `main.c` con parseo de línea de comandos.

### Etapa 7 — Llamadas al Sistema (SYS) y Disassembler
- Implementar `SYS 1` (READ) y `SYS 2` (WRITE) con formateo y prompt `[XXXX]`.
- Implementar `disassembler.c` con salida por buffer para flag `-d`.

### Etapa 8 — Suite de Pruebas Automatizadas y Validación
- Compilar y ejecutar `tests_runner`.
- Validar programas de ejemplo completos (ej.: programa de conteo de bits en 1).
