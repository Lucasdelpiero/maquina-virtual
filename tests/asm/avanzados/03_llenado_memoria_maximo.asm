; ==========================================================
; Test Avanzado 3: Llenado masivo de memoria casi al limite
; Llena 3500 palabras de 4 bytes (14.000 bytes de los 16 KiB)
; Verifica que se escriba y lea correctamente sin desbordar
; ==========================================================

mov edx, ds
mov eax, 0
mov ecx, 3500

bucle_llena: cmp ecx, 0
jnp fin_llena

; Escribir valor en memoria
mov [edx], eax

; Avanzar al siguiente entero de 4 bytes
add edx, 4
add eax, 1
sub ecx, 1
jmp bucle_llena

; Validar primera y ultima posicion escrita
fin_llena: mov eax, [0]
mov ebx, [edx-4]
stop
