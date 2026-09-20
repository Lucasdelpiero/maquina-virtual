; Programa complejo 1: Suma acumulativa de 1 a N (1..10 = 55)
mov ecx, 10
mov eax, 0
bucle: add eax, ecx
sub ecx, 1
jnz bucle
stop
