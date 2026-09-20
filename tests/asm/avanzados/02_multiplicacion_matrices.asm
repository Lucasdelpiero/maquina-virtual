; ==========================================================
; Test Avanzado 2: Multiplicacion de Matrices 2x2 en Memoria
; A = [[1, 2], [3, 4]] en [0..12]
; B = [[5, 6], [7, 8]] en [16..28]
; C = A x B = [[19, 22], [43, 50]] en [32..44]
; ==========================================================

; 1. Inicializar Matriz A
mov [0], 1
mov [4], 2
mov [8], 3
mov [12], 4

; 2. Inicializar Matriz B
mov [16], 5
mov [20], 6
mov [24], 7
mov [28], 8

; 3. Inicializar Matriz C en ceros
mov [32], 0
mov [36], 0
mov [40], 0
mov [44], 0

; 4. Algoritmo de Multiplicacion con 3 bucles while
; EAX: indice de fila i (0 a 1)
mov eax, 0

bucle_fila: cmp eax, 2
jnn fin_bucle_fila

; EBX: indice de columna j (0 a 1)
mov ebx, 0

bucle_col: cmp ebx, 2
jnn fin_bucle_col

; ECX: acumulador de suma para C[i][j]
mov ecx, 0

; EDX: indice k (0 a 1)
mov edx, 0

bucle_k: cmp edx, 2
jnn fin_bucle_k

; Calcular direccion de A[i][k] = (i * 2 + k) * 4
mov eex, eax
shl eex, 1
add eex, edx
shl eex, 2

; Puntero base DS + eex
mov efx, ds
add efx, eex
; Elemento A[i][k] en EEX
mov eex, [efx]

; Calcular direccion de B[k][j] = 16 + (k * 2 + j) * 4
mov efx, edx
shl efx, 1
add efx, ebx
shl efx, 2
add efx, 16

; Puntero base DS + efx
add efx, ds
; Elemento B[k][j] en EFX
mov efx, [efx]

; Producto A[i][k] * B[k][j]
mul eex, efx

; Sumar al acumulador ECX
add ecx, eex

; k++
add edx, 1
jmp bucle_k

; Guardar resultado en C[i][j] = 32 + (i * 2 + j) * 4
fin_bucle_k: mov eex, eax
shl eex, 1
add eex, ebx
shl eex, 2
add eex, 32

mov efx, ds
add efx, eex
mov [efx], ecx

; col++
add ebx, 1
jmp bucle_col

; row++
fin_bucle_col: add eax, 1
jmp bucle_fila

; 5. Cargar resultados en registros para validacion
fin_bucle_fila: mov eax, [32] ; Esperado: 19
mov ebx, [36] ; Esperado: 22
mov ecx, [40] ; Esperado: 43
mov edx, [44] ; Esperado: 50
stop
