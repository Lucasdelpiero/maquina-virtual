# Correcciones Realizadas - Ejercicio 12

Resumen de ajustes aplicados sobre los archivos del ejercicio para su correcta traducción y ejecución en la Máquina Virtual:

1. **Rótulo en línea sin instrucción (`cant_caminos.asm`)**:
   - La etiqueta `cant_fin:` se encontraba sola en una línea seguida únicamente por un comentario (`cant_fin: ; Epilogo`).
   - `vmt.exe` no registra rótulos en líneas en blanco o con solo comentarios, produciendo `ERROR: Símbolo inexistente: 'CANT_FIN'`.
   - Se unió el rótulo en la misma línea con la instrucción del epílogo: `cant_fin: MOV SP, BP`.

## ERRORES DE LOGICA

2. **Inclusión de la subrutina (`cant_caminos.asm`)**:
   - `cant_caminos` invocaba a `rec_mat` mediante `CALL rec_mat`, pero no incluía el archivo correspondiente.
   - Se añadió la directiva `\\INCLUDE "rec_mat.asm"`.

3. **Comentarios al final del archivo (`rec_mat.asm`)**:
   - Las notas de estudio al pie del archivo ("Claves a recordar de este ejercicio...") carecían del punto y coma `;` de comentario.
   - `vmt.exe` intentaba parsear las palabras como instrucciones (`ERROR: Mnemónico desconocido: 'Claves'`).
   - Se comentaron todas las líneas de notas con `;`.

4. **Programa de prueba y verificación (`main_ej12.asm`)**:
   - Se creó un driver de prueba que inicializa en `\\DATA` una matriz de adyacencia de 5x5 para un digrafo acíclico (DAG) con 5 vértices (0 a 4) y las aristas `0->1`, `0->2`, `1->3`, `1->4`, `2->3`, `3->4`.
   - Configura la estructura `GRAFO` (`CANT = 5`, puntero `MATRIZ`) e invoca `cant_caminos` de origen 0 a destino 4.
   - Verifica que `EAX` retorne exactamente 3 caminos (`0->1->4`, `0->1->3->4` y `0->2->3->4`).
