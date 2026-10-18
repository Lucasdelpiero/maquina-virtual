; Test: Salto condicional JP cuando el resultado es positivo estricto (N=0 y Z=0)
mov eax, 15
cmp eax, 10
jp es_positivo
mov ebx, 0
jmp fin
es_positivo: mov ebx, 1
fin: stop
