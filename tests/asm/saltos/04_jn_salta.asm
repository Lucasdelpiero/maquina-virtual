; Test: Salto condicional JN cuando el resultado es negativo (N=1)
mov eax, 5
sub eax, 10
jn es_negativo
mov ebx, 0
jmp fin
es_negativo: mov ebx, 1
fin: stop
