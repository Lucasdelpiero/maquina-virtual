; ==========================================================
; Test Avanzado 1: Ordenamiento Bubble Sort en Memoria
; Vector inicial en DS: [45, 12, 89, 3, 27, 50] (6 elementos)
; Vector ordenado esperado: [3, 12, 27, 45, 50, 89]
; ==========================================================

; 1. Carga inicial del vector en memoria
mov [0], 45
mov [4], 12
mov [8], 89
mov [12], 3
mov [16], 27
mov [20], 50

; 2. Inicializacion de variables del bucle
; EAX: Contador bucle externo (pasadas restantes = 5)
mov eax, 5

; Condicion while (EAX > 0)
bucle_externo: cmp eax, 0
jnp fin_bucle_externo

; EDX: Puntero al inicio del vector en datos
mov edx, ds

; ECX: Contador de comparaciones internas
mov ecx, eax

; Condicion while (ECX > 0)
bucle_interno: cmp ecx, 0
jnp fin_bucle_interno

; Cargar elemento actual y siguiente
mov ebx, [edx]
mov eex, [edx+4]

; Comparar EBX con EEX (EBX - EEX)
cmp ebx, eex
jnp no_intercambiar

; Si EBX > EEX, intercambiar en memoria
swap [edx], [edx+4]

no_intercambiar: add edx, 4

; Decrementar contador interno ECX
sub ecx, 1
jmp bucle_interno

fin_bucle_interno: sub eax, 1
jmp bucle_externo

; 3. Cargar elementos finales ordenados en registros para validacion
fin_bucle_externo: mov eax, [0]
mov ebx, [4]
mov ecx, [8]
mov edx, [12]
mov eex, [16]
mov efx, [20]
stop
