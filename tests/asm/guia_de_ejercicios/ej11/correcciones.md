# Correcciones Realizadas - Ejercicio 11

Resumen de ajustes aplicados sobre los archivos del ejercicio para su correcta traducción y ejecución en la Máquina Virtual:

1. **Extensión del archivo de subrutina**:
   - El archivo original `elim_valor` no poseía extensión `.asm`, por lo que `vmt.exe` lo rechazaba (`ERROR: No se ha especificado un archivo *.asm`).
   - Se renombró a `elim_valor.asm` y se normalizó la directiva a `\\INCLUDE "funciones.asm"`.

2. **Espaciado en rótulos (`funciones.asm`)**:
   - En la línea 134, el rótulo estaba pegado a la instrucción: `rec_elim_simple:MOV EDX, [BP+8]`.
   - El analizador léxico de `vmt.exe` requiere separación por espacios; se corrigió a `rec_elim_simple: MOV EDX, [BP+8]`.

## ERRORES DE LOGICA

3. **Protección ante árboles vacíos (`funciones.asm`)**:
   - En `rec_arbol`, al obtener `EBX = [EBX]` (puntero `ROOT*`), si el árbol era nulo (-1) la ejecución continuaba e intentaba acceder a campos de memoria inválidos.
   - Se añadió la verificación:
     ```assembly
     CMP EBX, NULL
     JZ rec_fin
     ```
   - Esto previene caídas por fallo de segmento al procesar árboles vacíos o podados.

4. **Programa de prueba y verificación (`main_ej11.asm`)**:
   - Se creó un driver de prueba que construye en `\\DATA` una lista enlazada de 3 árboles N-arios que contienen exactamente 5 nodos con valor 4:
     - Arbol 1: raíz con valor 4 (reemplazada por su primer hijo) y una hoja con valor 4.
     - Arbol 2: compuesto exclusivamente por nodos con valor 4 (al quedar vacío, su nodo de lista es eliminado).
     - Arbol 3: raíz con valor distinto y un nodo intermedio con valor 4.
   - Invoca a `elim_valor` con valor 4 y verifica que `EAX` retorne 5.
