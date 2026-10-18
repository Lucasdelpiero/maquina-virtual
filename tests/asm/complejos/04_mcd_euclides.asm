; Programa complejo 4: Algoritmo de Euclides para MCD(48, 18) = 6
mov eax, 48
mov ebx, 18

loop_mcd: div eax, ebx
mov eax, ebx
mov ebx, ac
cmp ebx, 0
jnz loop_mcd
stop
