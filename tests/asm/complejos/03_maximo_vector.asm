; Programa complejo 3: Busqueda del valor maximo en un vector (15, 82, 33, 64)
mov [0], 15
mov [4], 82
mov [8], 33
mov [12], 64

mov edx, ds
mov eax, [edx]
mov ecx, 3

sig: add edx, 4
mov ebx, [edx]
cmp ebx, eax
jnp no_cambiar
mov eax, ebx
no_cambiar: sub ecx, 1
jnz sig
stop
