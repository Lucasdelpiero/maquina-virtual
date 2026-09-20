; Test: Salto condicional JZ cuando el resultado es cero (Z=1)
mov eax, 10
sub eax, 10
jz exito
mov ebx, 99
exito: stop
