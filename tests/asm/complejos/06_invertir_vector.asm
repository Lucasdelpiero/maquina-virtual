; Programa complejo 6: Inversion de un vector en memoria usando SWAP memoria a memoria
mov [0], 10
mov [4], 20
mov [8], 30
mov [12], 40

mov edx, ds
mov ebx, ds
add ebx, 12

swap [edx], [ebx]
add edx, 4
sub ebx, 4
swap [edx], [ebx]

mov eax, [0]
mov ebx, [4]
mov ecx, [8]
mov edx, [12]
stop
