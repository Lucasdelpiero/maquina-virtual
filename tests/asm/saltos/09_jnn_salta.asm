; Test: JNN salta cuando el resultado no es negativo (>= 0)
mov eax, 10
add eax, 5
jnn exito
mov ebx, 99
jmp fin
exito: mov ebx, 1
fin: stop
