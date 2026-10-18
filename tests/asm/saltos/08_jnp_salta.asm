; Test: JNP salta cuando el resultado es menor o igual a cero (<= 0)
mov eax, 10
sub eax, 10
jnp exito
mov ebx, 99
jmp fin
exito: mov ebx, 1
fin: stop
