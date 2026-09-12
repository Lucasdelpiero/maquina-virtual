; Test: JV salta cuando hay desbordamiento (V=1)
ldl eax, 0xFFFF
ldh eax, 0x7FFF
mov ebx, eax
add eax, ebx
jv exito
mov ecx, 99
jmp fin
exito: mov ecx, 1
fin: stop
