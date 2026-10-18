; Test: JC salta cuando hay acarreo/prestamo (C=1)
mov eax, 3
sub eax, 5
jc exito
mov ebx, 99
jmp fin
exito: mov ebx, 1
fin: stop
