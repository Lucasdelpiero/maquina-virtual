; Programa complejo 5: Serie de Fibonacci calculando el 7mo termino (13)
mov eax, 0
mov ebx, 1
mov ecx, 7

loop_fib: mov edx, eax
add edx, ebx
mov eax, ebx
mov ebx, edx
sub ecx, 1
jnz loop_fib
stop
